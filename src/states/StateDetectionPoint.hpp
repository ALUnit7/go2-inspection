#pragma once
#include "core/StateBase.hpp"
class StateDetectionPoint : public StateBase {
public:
    void enter(RobotContext& ctx) override;
    int  update(RobotContext& ctx) override;
private:
    enum Phase { TURN_L, RECOGNIZE, ACT, TURN_R, DONE };
    Phase phase_    = TURN_L;
    int   ticks_    = 0;
    float yaw_start_= 0, yaw_accum_= 0;
};
