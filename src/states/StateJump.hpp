#pragma once
#include "core/StateBase.hpp"
#include <opencv2/core.hpp>

class StateJump : public StateBase {
public:
    explicit StateJump(int next_state) : next_state_(next_state) {}
    void enter(RobotContext& ctx) override;
    int  update(RobotContext& ctx) override;

private:
    enum Phase { ALIGN, JUMP, LAND };
    int   next_state_;
    Phase phase_      = ALIGN;
    int   wait_ticks_ = 0;
    int   align_ticks_= 0;
};
