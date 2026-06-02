#include "LineDetector.hpp"
#include <opencv2/imgproc.hpp>
#include <cmath>
#include <numeric>

bool LineDetector::getLineEdge(const cv::Mat& bin, int row, int startX,
                                int& outLeft, int& outRight) const {
    int cols = bin.cols;
    if (startX < 0 || startX >= cols) return false;
    outLeft = outRight = -1;
    uchar center = bin.at<uchar>(row, startX);

    if (center == 255) {
        for (int x = startX; x >= 0; x--)
            if (bin.at<uchar>(row, x) == 0) { outLeft = x + 1; break; }
        for (int x = startX; x < cols; x++)
            if (bin.at<uchar>(row, x) == 0) { outRight = x - 1; break; }
    } else {
        int nearL = -1, nearR = -1;
        for (int x = startX - 1; x >= 0; x--)
            if (bin.at<uchar>(row, x) == 255) { nearL = x; break; }
        for (int x = startX + 1; x < cols; x++)
            if (bin.at<uchar>(row, x) == 255) { nearR = x; break; }
        if (nearL == -1 && nearR == -1) return false;
        if (nearL != -1 && nearR == -1) {
            for (int x = nearL; x >= 0; x--)
                if (bin.at<uchar>(row, x) == 0) { outLeft = x + 1; break; }
            outRight = nearL;
        } else if (nearR != -1 && nearL == -1) {
            for (int x = nearR; x < cols; x++)
                if (bin.at<uchar>(row, x) == 0) { outRight = x - 1; break; }
            outLeft = nearR;
        } else {
            if (startX - nearL <= nearR - startX) {
                for (int x = nearL; x >= 0; x--)
                    if (bin.at<uchar>(row, x) == 0) { outLeft = x + 1; break; }
                outRight = nearL;
            } else {
                for (int x = nearR; x < cols; x++)
                    if (bin.at<uchar>(row, x) == 0) { outRight = x - 1; break; }
                outLeft = nearR;
            }
        }
    }
    if (outLeft  == -1) outLeft  = 0;
    if (outRight == -1) outRight = cols - 1;
    return true;
}

bool LineDetector::linearRegress(const std::vector<cv::Point>& pts,
                                  double& slope, double& intercept,
                                  std::vector<bool>& valid_mask) const {
    int n = (int)pts.size();
    if (n < 2) return false;

    // 第一次全量最小二乘
    auto regress = [](const std::vector<cv::Point>& p,
                      const std::vector<bool>& mask,
                      double& a, double& b) {
        double sx=0, sy=0, sxx=0, sxy=0; int cnt=0;
        for (int i=0;i<(int)p.size();i++) {
            if (!mask[i]) continue;
            double x=p[i].y, y=p[i].x;  // row→x, cx→y
            sx+=x; sy+=y; sxx+=x*x; sxy+=x*y; cnt++;
        }
        if (cnt<2) return false;
        double denom = cnt*sxx - sx*sx;
        if (std::abs(denom) < 1e-6) return false;
        a = (cnt*sxy - sx*sy) / denom;
        b = (sy - a*sx) / cnt;
        return true;
    };

    valid_mask.assign(n, true);
    double a, b;
    if (!regress(pts, valid_mask, a, b)) return false;

    // 剔除离群点（残差 > ransac_thresh_）
    for (int i = 0; i < n; i++) {
        double pred = a * pts[i].y + b;
        if (std::abs(pts[i].x - pred) > ransac_thresh_)
            valid_mask[i] = false;
    }

    // 第二次回归（只用内点）
    int inliers = std::count(valid_mask.begin(), valid_mask.end(), true);
    if (inliers < 2) {
        // 内点太少，退回全量结果
        valid_mask.assign(n, true);
        slope = a; intercept = b;
        return true;
    }

    return regress(pts, valid_mask, slope, intercept);
}

LineResult LineDetector::detectImpl(const cv::Mat& bgr, cv::Mat* vis) {
    cv::Mat gray, binary;
    cv::cvtColor(bgr, gray, cv::COLOR_BGR2GRAY);
    cv::threshold(gray, binary, thresh_, 255, cv::THRESH_BINARY_INV);

    cv::Mat kc = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(close_k_,close_k_));
    cv::Mat ko = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(open_k_, open_k_));
    cv::dilate(binary, binary, kc); cv::erode(binary, binary, kc);
    cv::erode(binary, binary, ko);  cv::dilate(binary, binary, ko);

    int h = binary.rows, w = binary.cols;
    int bot_row = h - 1;
    int top_row = (top_row_ < h) ? top_row_ : 0;

    std::vector<cv::Point> pts;
    int botL = 0, botR = w-1;

    // 底部组：top_row ~ bot_row，逐行跟随
    int start_x = (last_cx_ > 0) ? last_cx_ : w / 2;
    int cur_x   = start_x;
    int step = (n_rows_ > 1) ? (bot_row - top_row) / (n_rows_ - 1) : 1;
    for (int i = 0; i < n_rows_; i++) {
        int row = bot_row - i * step;
        if (row < top_row) break;
        int L, R;
        if (getLineEdge(binary, row, cur_x, L, R)) {
            int cx = (L + R) / 2;
            pts.push_back({cx, row});
            cur_x = cx;
            if (i == 0) { botL = L; botR = R; }
        }
    }

    // 顶部组：0 ~ top_row，从图像中心出发独立采样
    // 用于十字路口时纳入过了交叉点的正常直线点
    int top_step = (top_row > 0 && n_rows_ > 1) ? top_row / (n_rows_ - 1) : 1;
    int top_cx = w / 2;
    for (int i = 1; i < n_rows_; i++) {
        int row = top_row - i * top_step;
        if (row < 0) break;
        int L, R;
        if (getLineEdge(binary, row, top_cx, L, R)) {
            int cx = (L + R) / 2;
            pts.push_back({cx, row});
            top_cx = cx;
        }
    }

    if (pts.empty()) return {0, 0, 0, 0, false, false};

    // 线性回归
    double slope = 0, intercept = pts[0].x;
    std::vector<bool> valid_mask;
    bool reg_ok = (pts.size() >= 2) && linearRegress(pts, slope, intercept, valid_mask);

    // 底部行预测中点
    double bot_cx = slope * bot_row + intercept;
    last_cx_ = (int)bot_cx;

    // angle：斜率转角度（slope = dx/dy，与 atan2 一致）
    double angle  = -std::atan2(slope, 1.0) * 180.0 / CV_PI;
    double offset = -(bot_cx - w / 2.0);
    int line_width = botR - botL;

    // top_ok：内点数超过一半认为回归有效
    int inliers = reg_ok ? (int)std::count(valid_mask.begin(), valid_mask.end(), true) : 0;
    bool top_ok = inliers >= (int)(pts.size() / 2);

    if (vis) {
        // 画所有采样点
        for (int i = 0; i < (int)pts.size(); i++) {
            cv::Scalar c = (reg_ok && valid_mask[i]) ? cv::Scalar{0,255,0} : cv::Scalar{0,0,255};
            cv::circle(*vis, pts[i], 5, c, -1);
        }
        // 画回归直线（从 top_row 到 bot_row）
        if (reg_ok) {
            cv::Point p1((int)(slope * top_row + intercept), top_row);
            cv::Point p2((int)bot_cx, bot_row);
            cv::line(*vis, p1, p2, {255,0,0}, 2);
        }
        cv::line(*vis, {w/2,0}, {w/2,h}, {0,200,0}, 1);
        cv::putText(*vis, "Angle:" + std::to_string(angle).substr(0,6),
                    {20,40}, cv::FONT_HERSHEY_SIMPLEX, 1, {255,255,0}, 2);
        cv::putText(*vis, "Offset:" + std::to_string((int)offset),
                    {20,80}, cv::FONT_HERSHEY_SIMPLEX, 1, {0,255,255}, 2);
        cv::putText(*vis, "W:" + std::to_string(line_width) +
                    " IN:" + std::to_string(inliers) + "/" + std::to_string(pts.size()),
                    {20,120}, cv::FONT_HERSHEY_SIMPLEX, 0.8,
                    top_ok ? cv::Scalar{0,255,0} : cv::Scalar{0,0,255}, 2);
    }

    double length = std::abs(bot_row - top_row) / std::cos(slope);
    return {angle, offset, length, line_width, true, top_ok};
}

LineResult LineDetector::detect(const cv::Mat& bgr) {
    return detectImpl(bgr, nullptr);
}

LineResult LineDetector::detectAndDraw(cv::Mat& vis) {
    return detectImpl(vis, &vis);
}
