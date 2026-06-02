#include "SignDetector.hpp"
#include <opencv2/imgproc.hpp>
#include <algorithm>

SignResult SignDetector::detect(const cv::Mat& bgr, cv::Mat* vis) const {
    cv::Mat hsv, mask1, mask2, mask;
    cv::cvtColor(bgr, hsv, cv::COLOR_BGR2HSV);
    cv::inRange(hsv,
        cv::Scalar(cfg_.h1_lo, cfg_.s_lo, cfg_.v_lo),
        cv::Scalar(cfg_.h1_hi, cfg_.s_hi, cfg_.v_hi), mask1);
    cv::inRange(hsv,
        cv::Scalar(cfg_.h2_lo, cfg_.s_lo, cfg_.v_lo),
        cv::Scalar(cfg_.h2_hi, cfg_.s_hi, cfg_.v_hi), mask2);
    mask = mask1 | mask2;

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // 找最大轮廓
    int best = -1; double best_area = 0;
    for (int i = 0; i < (int)contours.size(); i++) {
        double a = cv::contourArea(contours[i]);
        if (a > best_area && a > cfg_.min_area) { best_area = a; best = i; }
    }
    if (best < 0) return {0, 0, false};

    cv::Point2f center; float radius;
    cv::minEnclosingCircle(contours[best], center, radius);

    if (vis) {
        cv::circle(*vis, center, (int)radius, {0,255,0}, 2);
        cv::circle(*vis, center, 3, {0,0,255}, -1);
    }

    // ROI 内黑白像素比
    cv::Rect roi(
        std::max(0, (int)(center.x - radius)),
        std::max(0, (int)(center.y - radius)),
        std::min(bgr.cols - std::max(0,(int)(center.x-radius)), (int)(2*radius)),
        std::min(bgr.rows - std::max(0,(int)(center.y-radius)), (int)(2*radius))
    );
    if (roi.width <= 0 || roi.height <= 0) return {0, 0, false};

    cv::Mat gray, binary;
    cv::cvtColor(bgr(roi), gray, cv::COLOR_BGR2GRAY);
    cv::threshold(gray, binary, 100, 255, cv::THRESH_BINARY);

    int white = cv::countNonZero(binary);
    int black = binary.total() - white;
    if (white == 0) return {0, 0, false};

    double ratio = black / (double)white;
    int type = (ratio > cfg_.ratio_thresh) ? 1 : 2;
    return {type, ratio, true};
}
