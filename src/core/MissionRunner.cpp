#include "MissionRunner.hpp"
#include "StateIDs.hpp"
#include "states/StateIdle.hpp"
#include "states/StateLineFollow.hpp"
#include "states/StateJump.hpp"
#include "states/StateObstacleAvoid.hpp"
#include "states/StateClimbStairs.hpp"
#include "states/StateVisionTask.hpp"
#include "states/StateArmControl.hpp"
#include <iostream>
#include <chrono>
#include <thread>

MissionRunner::MissionRunner(RobotContext ctx)
    : ctx_(std::move(ctx)), current_id_(STATE_IDLE)
{
    register_states();
}

void MissionRunner::register_states() {
    states_[STATE_IDLE]           = std::make_unique<StateIdle>();
    states_[STATE_LINE_FOLLOW]    = std::make_unique<StateLineFollow>();
    states_[STATE_JUMP]           = std::make_unique<StateJump>();
    states_[STATE_OBSTACLE_AVOID] = std::make_unique<StateObstacleAvoid>();
    states_[STATE_CLIMB_STAIRS]   = std::make_unique<StateClimbStairs>();
    states_[STATE_VISION_TASK]    = std::make_unique<StateVisionTask>();
    states_[STATE_ARM_CONTROL]    = std::make_unique<StateArmControl>();
}

void MissionRunner::transition(int next_id) {
    states_.at(current_id_)->exit(ctx_);
    std::cout << "[Mission] state " << current_id_ << " -> " << next_id << "\n";
    current_id_ = next_id;
    states_.at(current_id_)->enter(ctx_);
}

void MissionRunner::run() {
    states_.at(current_id_)->enter(ctx_);
    while (current_id_ != STATE_DONE) {
        int next = states_.at(current_id_)->update(ctx_);
        if (next != STATE_STAY)
            transition(next);
        std::this_thread::sleep_for(std::chrono::milliseconds(20)); // 50Hz
    }
    std::cout << "[Mission] done.\n";
}
