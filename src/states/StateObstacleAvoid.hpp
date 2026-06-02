#pragma once
#include "core/StateBase.hpp"

// 动态避障：调用 SDK FreeAvoid 模式，检测到通过标记后退出
class StateObstacleAvoid : public StateBase {
public:
    void enter(RobotContext& ctx) override;
    int  update(RobotContext& ctx) override;
    void exit(RobotContext& ctx) override;

private:
    int ticks_{0};
};
