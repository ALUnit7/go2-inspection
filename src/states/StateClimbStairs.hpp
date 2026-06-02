#pragma once
#include <cmath>
#include "core/StateBase.hpp"

class StateClimbStairs : public StateBase {
public:
    void enter(RobotContext& ctx) override;
    int  update(RobotContext& ctx) override;
    void exit(RobotContext& ctx) override;

private:
    int ticks_{0};
    int pitch_stable_ticks_{0};
};
