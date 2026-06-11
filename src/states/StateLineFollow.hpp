#pragma once
#include <opencv2/core.hpp>
#include "core/StateBase.hpp"
#include "LineFollowConfig.hpp"

class StateLineFollow : public StateBase {
public:
    explicit StateLineFollow(LineFollowConfig cfg) : cfg_(cfg) {}
    void enter(RobotContext& ctx) override;
    int  update(RobotContext& ctx) override;
    void exit(RobotContext& ctx) override;

private:
    LineFollowConfig cfg_;
    int  lost_frames_   = 0;
    int  corner_count_  = 0;
    bool in_corner_     = false;
    float yaw_start_    = 0;
    float yaw_accum_    = 0;
    float speed_        = 0.2f;
    float kp_           = 0.05f;
    float kp2_          = 0.003f;

    bool detectObstacle(const cv::Mat& binary, int w) const;
    bool detectRedCircle(const cv::Mat& bgr) const;
};
