#pragma once
#include <unordered_map>
#include <memory>
#include "StateBase.hpp"
#include "RobotContext.hpp"
class MissionRunner {
public:
    explicit MissionRunner(RobotContext ctx);
    void run();
private:
    RobotContext ctx_;
    std::unordered_map<int, std::unique_ptr<StateBase>> states_;
    int current_id_;
    void register_states();
    void transition(int next_id);
};
