#include "StateJump.hpp"
#include "core/RobotContext.hpp"
#include "core/StateIDs.hpp"
#include <iostream>
// ── 调参区 ───────────────────────────────────────────────────────────────────
static const int LAND_WAIT_TICKS = 100;  // 落地等待帧数（50Hz * 2s）
// ────────────────────────────────────────────────────────────────────────────
void StateJump::enter(RobotContext& ctx) {
    jumped_ = false; wait_ticks_ = 0;
    ctx.sport->stop();
    std::cout << "[Jump] FrontJump\n";
    ctx.sport->jump();
    jumped_ = true;
}
int StateJump::update(RobotContext& ctx) {
    if (!jumped_) return STATE_STAY;
    float pitch = ctx.monitor->rpy()[1] * 180.f / M_PI;
    if (std::abs(pitch) < 5.f) wait_ticks_++;
    else wait_ticks_ = 0;
    if (wait_ticks_ > LAND_WAIT_TICKS) {
        std::cout << "[Jump] landed\n";
        ctx.sport->freeWalk();
        return next_state_;
    }
    return STATE_STAY;
}
