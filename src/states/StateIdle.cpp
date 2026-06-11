#include "StateIdle.hpp"
#include "core/RobotContext.hpp"
#include "core/StateIDs.hpp"
#include <iostream>
void StateIdle::enter(RobotContext& ctx) {
    ctx.sport->stop();
    std::cout << "[Idle] Press Enter to start...\n";
}
int StateIdle::update(RobotContext& ctx) {
    // 等待回车键启动
    std::string s;
    std::getline(std::cin, s);
    ctx.memory = MissionMemory{};  // 清空记忆
    return STATE_LINE_FOLLOW_1;
}
