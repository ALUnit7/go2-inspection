#pragma once
#include <opencv2/core.hpp>

// 检测结果
// 1 = 黑色图案为主 → 1号放置平台
// 2 = 白色图案为主 → 2号放置平台
// 0 = 未检测到红色标志
struct SignResult {
    int    type;        // 0=未检测, 1=→1号平台, 2=→2号平台
    double ratio;       // black/white 像素比
    bool   valid;       // type != 0
};

struct HsvRange {
    int h1_lo=0,  h1_hi=15;   // 红色区间1（色调 0~15）
    int h2_lo=163,h2_hi=179;  // 红色区间2（色调 163~179）
    int s_lo=135, s_hi=255;
    int v_lo=132, v_hi=205;
    int min_area=500;          // 最小有效轮廓面积（像素）
    double ratio_thresh=1.2;   // 黑/白比阈值，>则为type1，否则type2
};

class SignDetector {
public:
    explicit SignDetector(const HsvRange& cfg = {}) : cfg_(cfg) {}

    // 检测标志，在 vis 上画圆（vis=nullptr 不画）
    SignResult detect(const cv::Mat& bgr, cv::Mat* vis = nullptr) const;

    HsvRange& config() { return cfg_; }

private:
    HsvRange cfg_;
};
