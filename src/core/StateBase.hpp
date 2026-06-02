#pragma once

struct RobotContext;

constexpr int STATE_STAY = -1;

class StateBase {
public:
    virtual ~StateBase() = default;
    virtual void enter(RobotContext& ctx) {}
    virtual int  update(RobotContext& ctx) = 0;  // 返回下一状态ID或STATE_STAY
    virtual void exit(RobotContext& ctx) {}
};
