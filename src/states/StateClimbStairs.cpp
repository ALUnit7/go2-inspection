#include "StateClimbStairs.hpp"
#include "core/RobotContext.hpp"
#include "core/StateIDs.hpp"
#include <iostream>

void StateClimbStairs::enter(RobotContext& ctx) {
    std::cout << "[ClimbStairs] enter\n";
    ticks_ = 0;
    pitch_stable_ticks_ = 0;
    ctx.sport->walkStair(true);  // 专用爬楼梯模式
    ctx.sport->move(ctx.cfg["tasks"]["climb_stairs"]["speed"].as<float>(0.2f), 0, 0);
}

int StateClimbStairs::update(RobotContext& ctx) {
    auto rpy = ctx.monitor->rpy();
    float pitch = rpy[1];

    ctx.sport->move(ctx.cfg["tasks"]["climb_stairs"]["speed"].as<float>(0.2f), 0, 0);

    // pitch 恢复平稳（绝对值 < 0.1 rad）持续 1s（50帧）认为台阶通过
    if (std::abs(pitch) < 0.1f)
        pitch_stable_ticks_++;
    else
        pitch_stable_ticks_ = 0;

    if (++ticks_ > 50 && pitch_stable_ticks_ > 50)
        return STATE_VISION_TASK;

    return STATE_STAY;
}

void StateClimbStairs::exit(RobotContext& ctx) {
    ctx.sport->walkStair(false);  // 退出爬楼梯模式，回到灵动
    ctx.sport->stop();
}
