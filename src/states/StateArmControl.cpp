#include "StateArmControl.hpp"
#include "core/RobotContext.hpp"
#include "core/StateIDs.hpp"
#include <iostream>

void StateArmControl::enter(RobotContext& ctx) {
    std::cout << "[ArmControl] enter\n";
    ctx.sport->stop();
    phase_ = CONNECT;
    arm_ = std::make_unique<ArmStub>();
}

int StateArmControl::update(RobotContext& ctx) {
    std::string ip   = ctx.cfg["tasks"]["arm"]["ip"].as<std::string>("");
    int         port = ctx.cfg["tasks"]["arm"]["port"].as<int>(0);
    switch (phase_) {
    case CONNECT:
        if (arm_->connect(ip, port)) { phase_ = GRAB; }
        break;
    case GRAB:
        if (arm_->grab())    { phase_ = RELEASE; }
        break;
    case RELEASE:
        if (arm_->release()) { arm_->home(); phase_ = DONE; }
        break;
    case DONE:
        return STATE_DONE;
    }
    return STATE_STAY;
}
