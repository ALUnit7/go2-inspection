#pragma once
#include <memory>
constexpr int STATE_STAY = -1;
struct RobotContext;
class StateBase {
public:
    virtual ~StateBase() = default;
    virtual void enter(RobotContext&) {}
    virtual int  update(RobotContext&) = 0;
    virtual void exit(RobotContext&)   {}
};
