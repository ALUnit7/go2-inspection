#include "StateJump.hpp"
#include "core/RobotContext.hpp"
#include "core/StateIDs.hpp"
#include <iostream>

void StateJump::enter(RobotContext& ctx) {
    std::cout << "[Jump] enter\n";
    phase_ = APPROACH;
    wait_ticks_ = 0;
    next_state_ = ctx.cfg["jump"]["next_state"].as<int>(STATE_LINE_FOLLOW);
    ctx.sport->freeWalk();
}

int StateJump::update(RobotContext& ctx) {
    float jump_dist = ctx.cfg["tasks"]["jump"]["jump_trigger_dist"].as<float>(0.5f);
    float approach  = ctx.cfg["tasks"]["jump"]["approach_speed"].as<float>(0.4f);

    switch (phase_) {
    case APPROACH: {
        ctx.sport->move(approach, 0, 0);
        // 用雷达前向距离触发跳跃
        float front_range = ctx.monitor->frontRange();
        if (front_range > 0 && front_range < jump_dist) {
            ctx.sport->stop();
            ctx.sport->jump();
            phase_ = JUMPING;
            wait_ticks_ = 0;
            std::cout << "[Jump] triggered at range=" << front_range << "\n";
        }
        break;
    }
    case JUMPING:
        // 等待跳跃动作完成（error_code 离开 1008）
        if (ctx.monitor->errorCode() != 1008 && ++wait_ticks_ > 10) {
            phase_ = LANDING;
            wait_ticks_ = 0;
        }
        break;
    case LANDING:
        // 等待落地稳定（pitch 恢复平稳）
        if (std::abs(ctx.monitor->rpy()[1]) < 0.15f && ++wait_ticks_ > 20) {
            ctx.sport->freeWalk();
            return next_state_;
        }
        break;
    }
    return STATE_STAY;
}
