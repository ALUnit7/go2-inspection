#pragma once
#include "core/StateBase.hpp"

// 跳跃障碍：接近障碍物（雷达前向距离 < 阈值）-> FrontJump -> 等待落地
class StateJump : public StateBase {
public:
    void enter(RobotContext& ctx) override;
    int  update(RobotContext& ctx) override;

private:
    enum Phase { APPROACH, JUMPING, LANDING };
    Phase phase_{APPROACH};
    int   wait_ticks_{0};
    int   next_state_{0};  // 跳完后进入哪个状态
};
