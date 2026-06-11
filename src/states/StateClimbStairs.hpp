#pragma once
#include "core/StateBase.hpp"
class StateClimbStairs : public StateBase {
public:
    void enter(RobotContext& ctx) override;
    int  update(RobotContext& ctx) override;
    void exit(RobotContext& ctx) override;
private:
    enum Phase { CLIMB_UP, TURN_LEFT, CLIMB_DOWN, DONE };
    Phase phase_           = CLIMB_UP;
    int   stable_cnt_      = 0;
    bool  climbing_started_= false;  // pitch 已升高，确认开始上/下台阶
    float yaw_start_       = 0;
    float yaw_accum_       = 0;
};
