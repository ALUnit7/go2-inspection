#pragma once
#include "core/StateBase.hpp"
class StateReturnHome : public StateBase {
public:
    void enter(RobotContext& ctx) override;
    int  update(RobotContext& ctx) override;
    void exit(RobotContext& ctx) override;
private:
    int ticks_ = 0;
};
