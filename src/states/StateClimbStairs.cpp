#include "StateClimbStairs.hpp"
#include "core/RobotContext.hpp"
#include "core/StateIDs.hpp"
#include <iostream>
#include <cmath>
// ── 调参区 ───────────────────────────────────────────────────────────────────
static const float CLIMB_SPEED     = 0.08f;
static const float TURN_SPEED_S    = 0.4f;
static const float PITCH_STABLE    = 3.0f;   // pitch 稳定阈值(°)
static const int   STABLE_FRAMES   = 20;
static const float YAW_TURN        = 85.0f;
// ────────────────────────────────────────────────────────────────────────────
void StateClimbStairs::enter(RobotContext& ctx) {
    phase_ = CLIMB_UP; stable_cnt_ = 0;
    ctx.sport->freeWalk();
    ctx.sport->move(CLIMB_SPEED, 0, 0);
    std::cout << "[Stairs] enter CLIMB_UP\n";
}
int StateClimbStairs::update(RobotContext& ctx) {
    auto rpy = ctx.monitor->rpy();
    float pitch = rpy[1] * 180.f / M_PI;
    float yaw   = rpy[2] * 180.f / M_PI;
    switch (phase_) {
    case CLIMB_UP:
        ctx.sport->move(CLIMB_SPEED, 0, 0);
        if (std::abs(pitch) < PITCH_STABLE) stable_cnt_++;
        else stable_cnt_ = 0;
        if (stable_cnt_ > STABLE_FRAMES) {
            ctx.sport->stop();
            yaw_start_ = yaw; yaw_accum_ = 0; stable_cnt_ = 0;
            phase_ = TURN_LEFT;
            std::cout << "[Stairs] top, TURN_LEFT\n";
        }
        break;
    case TURN_LEFT: {
        ctx.sport->move(0, 0, TURN_SPEED_S);
        float d = yaw - yaw_start_;
        if (d > 180) d -= 360; if (d < -180) d += 360;
        yaw_accum_ = std::abs(d);
        if (yaw_accum_ >= YAW_TURN) {
            ctx.sport->stop(); stable_cnt_ = 0;
            phase_ = CLIMB_DOWN;
            std::cout << "[Stairs] turned, CLIMB_DOWN\n";
        }
        break;
    }
    case CLIMB_DOWN:
        ctx.sport->move(CLIMB_SPEED, 0, 0);
        if (std::abs(pitch) < PITCH_STABLE) stable_cnt_++;
        else stable_cnt_ = 0;
        if (stable_cnt_ > STABLE_FRAMES) {
            ctx.sport->stop();
            phase_ = DONE;
            std::cout << "[Stairs] done\n";
        }
        break;
    case DONE:
        ctx.sport->freeWalk();
        return STATE_LINE_FOLLOW_4;
    }
    return STATE_STAY;
}
void StateClimbStairs::exit(RobotContext& ctx) { ctx.sport->stop(); }
