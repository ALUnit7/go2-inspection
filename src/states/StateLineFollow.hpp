#pragma once
#include <opencv2/core.hpp>
#include "core/StateBase.hpp"

class StateLineFollow : public StateBase {
public:
    void enter(RobotContext& ctx) override;
    int  update(RobotContext& ctx) override;
    void exit(RobotContext& ctx) override;

private:
    int   frames_lost_{0};
    float speed_{0.3f};
    float kp_{0.005f};

    bool compute_error(const cv::Mat& frame, float& error);
};
