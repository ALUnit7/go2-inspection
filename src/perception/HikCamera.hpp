#pragma once
#include <opencv2/core.hpp>
#include "MvCameraControl.h"

struct HikConfig {
    float exposure_us  = -1;   // 曝光时间(微秒)，-1=自动
    float gain         = -1;   // 增益，-1=自动
    bool  auto_wb      = true; // 自动白平衡
};

class HikCamera {
public:
    ~HikCamera() { close(); }

    bool init(const HikConfig& cfg = {});
    bool grab(cv::Mat& frame);
    void setExposure(float us);   // 实时修改曝光（us>0固定，<=0自动）
    void setGain(float gain);     // 实时修改增益（>=0固定，<0自动）
    void close();

private:
    void*        handle_{nullptr};
    unsigned int width_{0}, height_{0};
    std::vector<unsigned char> buf_;
};
