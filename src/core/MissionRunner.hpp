#pragma once
#include <memory>
#include <unordered_map>
#include "StateBase.hpp"
#include "RobotContext.hpp"

// 顶层状态机驱动器：注册所有状态，循环调用 update()
class MissionRunner {
public:
    explicit MissionRunner(RobotContext ctx);
    void run();  // 阻塞运行直到 STATE_DONE

private:
    RobotContext ctx_;
    std::unordered_map<int, std::unique_ptr<StateBase>> states_;
    int current_id_;

    void register_states();
    void transition(int next_id);
};
