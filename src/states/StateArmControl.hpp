#pragma once
#include <memory>
#include "core/StateBase.hpp"
#include "arm/ArmInterface.hpp"

class StateArmControl : public StateBase {
public:
    void enter(RobotContext& ctx) override;
    int  update(RobotContext& ctx) override;

private:
    enum Phase { CONNECT, GRAB, RELEASE, DONE };
    Phase phase_{CONNECT};
    std::unique_ptr<ArmInterface> arm_;
};
