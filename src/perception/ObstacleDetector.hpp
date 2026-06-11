#pragma once
#include <opencv2/core.hpp>

struct ObstacleConfig {
    int   near_row   = 1000;  // 近处采样行（越大越靠近底部，触发越晚）
    int   far_row    = 700;   // 远处采样行
    float white_thresh = 0.10f; // ROI内白色占比低于此值触发
    int   confirm_frames = 3;   // 连续N帧才确认，防误触
};

class ObstacleDetector {
public:
    explicit ObstacleDetector(const ObstacleConfig& cfg = {}) : cfg_(cfg), counter_(0) {}

    // binary: THRESH_BINARY_INV 后的二值图
    // start_x: 扫描起点（传入 LineDetector 的 last_cx_，-1=用图像中心）
    // vis: 不为空则画出 ROI 梯形
    bool detect(const cv::Mat& binary, cv::Mat* vis = nullptr, int start_x = -1);

    void reset() { counter_ = 0; }
    ObstacleConfig& config() { return cfg_; }

private:
    ObstacleConfig cfg_;
    int counter_;
};
