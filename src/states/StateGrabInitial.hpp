#pragma once
#include "core/StateBase.hpp"
class StateGrabInitial : public StateBase {
public:
    void enter(RobotContext& ctx) override;
    int  update(RobotContext& ctx) override;
private:
    int  phase_  = 0;
    int  ticks_  = 0;
    float yaw_start_=0, yaw_accum_=0;
};
