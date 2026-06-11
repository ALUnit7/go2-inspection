#pragma once
#include "core/StateBase.hpp"
class StateTransfer : public StateBase {
public:
    void enter(RobotContext& ctx) override;
    int  update(RobotContext& ctx) override;
private:
    int ticks_ = 0;
};
