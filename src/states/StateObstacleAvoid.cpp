#include "StateObstacleAvoid.hpp"
#include "core/RobotContext.hpp"
#include "core/StateIDs.hpp"
#include <iostream>

void StateObstacleAvoid::enter(RobotContext& ctx) {
    std::cout << "[ObstacleAvoid] enter\n";
    ticks_ = 0;
    ctx.sport->move(0, 0, 0); // 先停
    // FreeAvoid 模式：移动时自动绕障，静止时对前方物体闪避
    // 通过 SportClient 直接调用（SDK 封装中预留）
}

int StateObstacleAvoid::update(RobotContext& ctx) {
    ctx.sport->move(0.3f, 0, 0);
    // TODO: 检测到避障区终点标记（视觉/里程计）时切换
    if (++ticks_ > 300) // 临时：6s后进入下一阶段
        return STATE_CLIMB_STAIRS;
    return STATE_STAY;
}

void StateObstacleAvoid::exit(RobotContext& ctx) {
    ctx.sport->stop();
}
