#pragma once
#include <opencv2/core.hpp>
#include <vector>

struct TagResult {
    int    id;
    double distance;  // 估算距离(m)，基于针孔模型
};

// ArUco DICT_6X6_250 检测 + 距离估算
class ArucoDetector {
public:
    // tag_size:      Tag 黑色区域物理边长(m)
    // focal_length:  相机焦距(px)，需标定
    explicit ArucoDetector(double tag_size = 0.0714, double focal_length = 600.0);

    // 检测并返回所有识别到的 Tag，在 vis 上画框（vis 为空则不画）
    std::vector<TagResult> detect(const cv::Mat& frame, cv::Mat* vis = nullptr);

private:
    double tag_size_;
    double focal_length_;
    cv::Ptr<cv::aruco::Dictionary>        dict_;
    cv::Ptr<cv::aruco::DetectorParameters> params_;
};
