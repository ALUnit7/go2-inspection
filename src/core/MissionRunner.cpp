#include "MissionRunner.hpp"
#include "StateIDs.hpp"
#include "states/StateIdle.hpp"
#include "states/StateLineFollow.hpp"
#include "states/StateJump.hpp"
#include "states/StateObstacleAvoid.hpp"
#include "states/StateClimbStairs.hpp"
#include "states/StateGrabInitial.hpp"
#include "states/StateTransfer.hpp"
#include "states/StateDetectionPoint.hpp"
#include "states/StatePlacePlatform.hpp"
#include "states/StateReturnHome.hpp"
#include <iostream>
#include <chrono>
#include <thread>

MissionRunner::MissionRunner(RobotContext ctx)
    : ctx_(std::move(ctx)), current_id_(STATE_IDLE)
{
    register_states();
}

void MissionRunner::register_states() {
    using namespace std;
    states_[STATE_IDLE]           = make_unique<StateIdle>();

    // ── 8段寻迹，各自配置退出条件 ─────────────────────────────────────────
    states_[STATE_LINE_FOLLOW_1]  = make_unique<StateLineFollow>(LineFollowConfig{
        .exit_on_obstacle = true, .next_state = STATE_JUMP_START});
    states_[STATE_LINE_FOLLOW_2]  = make_unique<StateLineFollow>(LineFollowConfig{
        .exit_on_line_lost = true, .next_state = STATE_OBSTACLE_AVOID});
    states_[STATE_LINE_FOLLOW_3]  = make_unique<StateLineFollow>(LineFollowConfig{
        .exit_on_tag = true, .target_tag_id = 0, .next_state = STATE_CLIMB_STAIRS});
    states_[STATE_LINE_FOLLOW_4]  = make_unique<StateLineFollow>(LineFollowConfig{
        .exit_on_odometry = true, .radar_side = 'L', .next_state = STATE_GRAB_INITIAL});
    states_[STATE_LINE_FOLLOW_5]  = make_unique<StateLineFollow>(LineFollowConfig{
        .exit_on_corner = true, .target_corners = 2, .next_state = STATE_TRANSFER});
    states_[STATE_LINE_FOLLOW_6]  = make_unique<StateLineFollow>(LineFollowConfig{
        .exit_on_red_circle = true, .next_state = STATE_DETECTION_POINT});
    states_[STATE_LINE_FOLLOW_7]  = make_unique<StateLineFollow>(LineFollowConfig{
        .exit_on_odometry = true, .radar_side = 'N', .next_state = STATE_PLACE_PLATFORM});
    states_[STATE_LINE_FOLLOW_8]  = make_unique<StateLineFollow>(LineFollowConfig{
        .exit_on_obstacle = true, .next_state = STATE_JUMP_END});

    states_[STATE_JUMP_START]     = make_unique<StateJump>(STATE_LINE_FOLLOW_2);
    states_[STATE_OBSTACLE_AVOID] = make_unique<StateObstacleAvoid>();
    states_[STATE_CLIMB_STAIRS]   = make_unique<StateClimbStairs>();
    states_[STATE_GRAB_INITIAL]   = make_unique<StateGrabInitial>();
    states_[STATE_TRANSFER]       = make_unique<StateTransfer>();
    states_[STATE_DETECTION_POINT]= make_unique<StateDetectionPoint>();
    states_[STATE_PLACE_PLATFORM] = make_unique<StatePlacePlatform>();
    states_[STATE_JUMP_END]       = make_unique<StateJump>(STATE_RETURN_HOME);
    states_[STATE_RETURN_HOME]    = make_unique<StateReturnHome>();
}

void MissionRunner::transition(int next_id) {
    states_.at(current_id_)->exit(ctx_);
    std::cout << "[Mission] " << current_id_ << " -> " << next_id << "\n";
    current_id_ = next_id;
    states_.at(current_id_)->enter(ctx_);
}

void MissionRunner::run() {
    states_.at(current_id_)->enter(ctx_);
    while (current_id_ != STATE_DONE) {
        int next = states_.at(current_id_)->update(ctx_);
        if (next != STATE_STAY) transition(next);
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    std::cout << "[Mission] done.\n";
}
