#pragma once
#include <opencv2/core.hpp>

struct ObstacleConfig {
    int   near_row      = 1000;
    int   far_row       = 700;
    float white_thresh  = 0.10f;
    int   confirm_frames= 3;
};

class ObstacleDetector {
public:
    explicit ObstacleDetector(const ObstacleConfig& cfg = {}) : cfg_(cfg), counter_(0) {}

    // binary:   THRESH_BINARY_INV 二值图
    // line_cx:  LineDetector.lastCx()，线中心 x（复用线检测的扫描逻辑）
    // vis:      可视化输出
    bool detect(const cv::Mat& binary, cv::Mat* vis = nullptr, int line_cx = -1);

    void reset() { counter_ = 0; }
    ObstacleConfig& config() { return cfg_; }

private:
    ObstacleConfig cfg_;
    int counter_;

    // 复用 LineDetector 同款扫描逻辑：从 start_x 出发找白色区域左右边界
    bool scanRow(const cv::Mat& bin, int row, int start_x, int& L, int& R) const;
};
