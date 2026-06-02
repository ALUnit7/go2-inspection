#pragma once
#include <memory>
#include <yaml-cpp/yaml.h>
#include "robot/SportController.hpp"
#include "robot/StateMonitor.hpp"
#include "perception/CameraClient.hpp"

// 所有模块共享的上下文，通过引用传递给每个状态
struct RobotContext {
    YAML::Node                       cfg;
    std::shared_ptr<SportController> sport;
    std::shared_ptr<StateMonitor>    monitor;
    std::shared_ptr<CameraClient>    camera;
};
