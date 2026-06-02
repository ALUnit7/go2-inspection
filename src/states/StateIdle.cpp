#include "StateIdle.hpp"
#include "core/RobotContext.hpp"
#include "core/StateIDs.hpp"
#include <iostream>

void StateIdle::enter(RobotContext& ctx) {
    std::cout << "[Idle] enter\n";
    ctx.sport->stop();
}

int StateIdle::update(RobotContext& ctx) {
    // TODO: 等待外部触发信号（按键/网络指令）
    // 当前直接进入寻迹
    return STATE_LINE_FOLLOW;
}
