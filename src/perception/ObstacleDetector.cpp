#include "ObstacleDetector.hpp"
#include <opencv2/imgproc.hpp>

bool ObstacleDetector::detect(const cv::Mat& binary, cv::Mat* vis) {
    int h = binary.rows, w = binary.cols;
    int nr = std::min(cfg_.near_row, h - 1);
    int fr = std::min(cfg_.far_row,  nr - 1);
    int cx = w / 2;

    // 在两行上找线的左右边界
    auto scan_row = [&](int row, int& L, int& R) {
        L = 0; R = w - 1;
        // 从中心向左找左边界
        for (int x = cx; x >= 0; x--)
            if (binary.at<uchar>(row, x) == 0) { L = x + 1; break; }
        // 从中心向右找右边界
        for (int x = cx; x < w; x++)
            if (binary.at<uchar>(row, x) == 0) { R = x - 1; break; }
    };

    int nL, nR, fL, fR;
    scan_row(nr, nL, nR);
    scan_row(fr, fL, fR);

    // 梯形 ROI 掩码
    cv::Mat mask = cv::Mat::zeros(binary.size(), CV_8UC1);
    std::vector<cv::Point> poly = {
        {fL, fr}, {fR, fr}, {nR, nr}, {nL, nr}
    };
    cv::fillPoly(mask, std::vector<std::vector<cv::Point>>{poly}, 255);

    // ROI 内白色（255）像素占比
    int roi_total = cv::countNonZero(mask);
    if (roi_total == 0) { counter_ = 0; return false; }

    cv::Mat roi_white;
    cv::bitwise_and(binary, mask, roi_white);
    int white_px = cv::countNonZero(roi_white);
    float ratio  = (float)white_px / roi_total;

    bool triggered = ratio < cfg_.white_thresh;
    counter_ = triggered ? counter_ + 1 : 0;

    if (vis) {
        cv::Scalar color = (counter_ >= cfg_.confirm_frames) ? cv::Scalar{0,0,255} : cv::Scalar{0,255,255};
        cv::polylines(*vis, std::vector<std::vector<cv::Point>>{poly}, true, color, 2);
        char buf[64];
        snprintf(buf, sizeof(buf), "white=%.2f cnt=%d", ratio, counter_);
        cv::putText(*vis, buf, {fL, fr - 10}, cv::FONT_HERSHEY_SIMPLEX, 0.7, color, 2);
    }

    return counter_ >= cfg_.confirm_frames;
}
