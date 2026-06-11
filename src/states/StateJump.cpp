#include "StateJump.hpp"
#include "core/RobotContext.hpp"
#include "core/StateIDs.hpp"
#include "perception/LineDetector.hpp"
#include <iostream>
#include <cmath>
// ── 调参区 ───────────────────────────────────────────────────────────────────
static const int   LAND_WAIT_TICKS  = 100;   // 落地稳定帧数（50Hz*2s）
static const float ALIGN_OFFSET_THR = 30.f;  // 对齐阈值：offset<此值认为居中（像素）
static const int   ALIGN_MAX_TICKS  = 75;    // 最长对齐等待帧数（防卡死）
static const float ALIGN_KP2        = 0.003f;// 对齐时 vy 增益
// ────────────────────────────────────────────────────────────────────────────

void StateJump::enter(RobotContext& ctx) {
    phase_ = ALIGN; wait_ticks_ = align_ticks_ = 0;
    ctx.sport->stop();
    std::cout << "[Jump] aligning before jump...\n";
}

int StateJump::update(RobotContext& ctx) {
    switch (phase_) {
    case ALIGN: {
        // 用相机检测线 offset，vy 横向归中
        cv::Mat frame;
        if (ctx.camera->grab(frame)) {
            LineDetector det(67, 800, 12, 5);
            LineResult r = det.detect(frame);
            if (r.valid) {
                float vy = (float)(r.offset * ALIGN_KP2);
                ctx.sport->move(0, vy, 0);  // 只横向移动，不前进
                if (std::abs(r.offset) < ALIGN_OFFSET_THR)
                    align_ticks_++;
                else
                    align_ticks_ = 0;
            }
        }
        // 对齐完成或超时 → 跳跃
        if (align_ticks_ > 10 || ++wait_ticks_ > ALIGN_MAX_TICKS) {
            ctx.sport->stop();
            std::cout << "[Jump] FrontJump\n";
            ctx.sport->jump();
            phase_ = LAND; wait_ticks_ = 0;
        }
        break;
    }
    case LAND: {
        float pitch = ctx.monitor->rpy()[1] * 180.f / M_PI;
        if (std::abs(pitch) < 5.f) wait_ticks_++;
        else wait_ticks_ = 0;
        if (wait_ticks_ > LAND_WAIT_TICKS) {
            std::cout << "[Jump] landed\n";
            ctx.sport->freeWalk();
            return next_state_;
        }
        break;
    }
    default: break;
    }
    return STATE_STAY;
}
