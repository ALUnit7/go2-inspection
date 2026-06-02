#pragma once
#include "core/StateBase.hpp"
#include "perception/Detector.hpp"

// 视觉识别任务：抓取摄像头图像，YOLO推理，根据识别结果决策
class StateVisionTask : public StateBase {
public:
    void enter(RobotContext& ctx) override;
    int  update(RobotContext& ctx) override;

private:
    Detector detector_;
    bool     model_loaded_{false};
    int      confirm_count_{0}; // 连续确认帧数
};
