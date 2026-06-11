#include "StateReturnHome.hpp"
#include "core/RobotContext.hpp"
#include "core/StateIDs.hpp"
#include "perception/LineDetector.hpp"
#include <iostream>
// ── 调参区 ───────────────────────────────────────────────────────────────────
static const int HOME_TIMEOUT = 500;  // 停靠超时帧数（待现场调整）
// ────────────────────────────────────────────────────────────────────────────
void StateReturnHome::enter(RobotContext& ctx) {
    ticks_=0; ctx.sport->freeWalk();
    std::cout << "[ReturnHome] enter\n";
}
int StateReturnHome::update(RobotContext& ctx) {
    cv::Mat frame; ctx.camera->grab(frame);
    if (!frame.empty()) {
        LineDetector det(67,800,12,5);
        LineResult r = det.detect(frame);
        if (r.valid) {
            float vyaw = -(float)(r.angle * 0.05f);
            float vy   =  (float)(r.offset * 0.003f);
            ctx.sport->move(0.15f, vy, vyaw);
        }
    }
    // 里程计/计时停靠（TODO: 用实际停靠条件替换）
    if (++ticks_ > HOME_TIMEOUT) {
        ctx.sport->stop();
        std::cout << "[ReturnHome] stopped at home\n";
        return STATE_DONE;
    }
    return STATE_STAY;
}
void StateReturnHome::exit(RobotContext& ctx) { ctx.sport->stop(); }
