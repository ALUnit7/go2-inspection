#pragma once
#include "core/StateBase.hpp"

// 等待启动信号（当前：直接跳转到寻迹）
class StateIdle : public StateBase {
public:
    void enter(RobotContext& ctx) override;
    int  update(RobotContext& ctx) override;
};
