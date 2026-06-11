#include "ObstacleDetector.hpp"
#include <opencv2/imgproc.hpp>

// 与 LineDetector::getLineEdge 完全相同的逻辑
bool ObstacleDetector::scanRow(const cv::Mat& bin, int row, int start_x, int& L, int& R) const {
    int cols = bin.cols;
    if (start_x < 0 || start_x >= cols) return false;
    L = R = -1;
    uchar center = bin.at<uchar>(row, start_x);

    if (center == 255) {
        for (int x = start_x; x >= 0; x--)
            if (bin.at<uchar>(row, x) == 0) { L = x + 1; break; }
        for (int x = start_x; x < cols; x++)
            if (bin.at<uchar>(row, x) == 0) { R = x - 1; break; }
    } else {
        int nearL=-1, nearR=-1;
        for (int x=start_x-1; x>=0; x--)
            if (bin.at<uchar>(row,x)==255) { nearL=x; break; }
        for (int x=start_x+1; x<cols; x++)
            if (bin.at<uchar>(row,x)==255) { nearR=x; break; }
        if (nearL==-1 && nearR==-1) return false;
        if (nearL!=-1 && nearR==-1) {
            for (int x=nearL;x>=0;x--) if(bin.at<uchar>(row,x)==0){L=x+1;break;}
            R=nearL;
        } else if (nearR!=-1 && nearL==-1) {
            for (int x=nearR;x<cols;x++) if(bin.at<uchar>(row,x)==0){R=x-1;break;}
            L=nearR;
        } else {
            if (start_x-nearL <= nearR-start_x) {
                for (int x=nearL;x>=0;x--) if(bin.at<uchar>(row,x)==0){L=x+1;break;}
                R=nearL;
            } else {
                for (int x=nearR;x<cols;x++) if(bin.at<uchar>(row,x)==0){R=x-1;break;}
                L=nearR;
            }
        }
    }
    if (L==-1) L=0;
    if (R==-1) R=cols-1;
    return true;
}

bool ObstacleDetector::detect(const cv::Mat& binary, cv::Mat* vis, int line_cx) {
    int h = binary.rows, w = binary.cols;
    int nr = std::min(cfg_.near_row, h-1);
    int fr = std::min(cfg_.far_row,  nr-1);
    int cx = (line_cx>0 && line_cx<w) ? line_cx : w/2;

    int nL, nR, fL, fR;
    scanRow(binary, nr, cx,            nL, nR);
    scanRow(binary, fr, (nL+nR)/2,     fL, fR);  // 远处从近处中点出发

    std::vector<cv::Point> poly = {{fL,fr},{fR,fr},{nR,nr},{nL,nr}};
    cv::Mat mask = cv::Mat::zeros(binary.size(), CV_8UC1);
    cv::fillPoly(mask, std::vector<std::vector<cv::Point>>{poly}, 255);

    int roi_total = cv::countNonZero(mask);
    if (roi_total == 0) { counter_=0; return false; }

    cv::Mat roi_white;
    cv::bitwise_and(binary, mask, roi_white);
    float ratio = (float)cv::countNonZero(roi_white) / roi_total;

    bool triggered = ratio < cfg_.white_thresh;
    counter_ = triggered ? counter_+1 : 0;

    if (vis) {
        cv::Scalar color = (counter_>=cfg_.confirm_frames) ? cv::Scalar{0,0,255} : cv::Scalar{0,255,255};
        cv::polylines(*vis, std::vector<std::vector<cv::Point>>{poly}, true, color, 2);
        char buf[64]; snprintf(buf,sizeof(buf),"white=%.2f cnt=%d",ratio,counter_);
        cv::putText(*vis, buf, {fL,fr-10}, cv::FONT_HERSHEY_SIMPLEX, 0.7, color, 2);
    }
    return counter_ >= cfg_.confirm_frames;
}
