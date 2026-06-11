#pragma once
#include "core/StateBase.hpp"
class StateJump : public StateBase {
public:
    explicit StateJump(int next_state) : next_state_(next_state) {}
    void enter(RobotContext& ctx) override;
    int  update(RobotContext& ctx) override;
private:
    int  next_state_;
    int  wait_ticks_ = 0;
    bool jumped_     = false;
};
