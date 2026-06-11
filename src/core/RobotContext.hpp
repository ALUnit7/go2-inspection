#pragma once
#include <memory>
#include <yaml-cpp/yaml.h>
#include "MissionMemory.hpp"
#include "robot/SportController.hpp"
#include "robot/StateMonitor.hpp"
#include "perception/CameraClient.hpp"
struct RobotContext {
    YAML::Node                       cfg;
    std::shared_ptr<SportController> sport;
    std::shared_ptr<StateMonitor>    monitor;
    std::shared_ptr<CameraClient>    camera;
    MissionMemory                    memory;
};
