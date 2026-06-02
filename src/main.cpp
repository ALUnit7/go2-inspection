#include <iostream>
#include <unitree/robot/channel/channel_factory.hpp>
#include <yaml-cpp/yaml.h>
#include "core/MissionRunner.hpp"
#include "core/RobotContext.hpp"

int main(int argc, char** argv) {
    std::string cfg_path = (argc > 1) ? argv[1] : "config/mission.yaml";

    YAML::Node cfg;
    try {
        cfg = YAML::LoadFile(cfg_path);
    } catch (const std::exception& e) {
        std::cerr << "Failed to load config: " << e.what() << "\n";
        return 1;
    }

    std::string iface = cfg["network"]["interface"].as<std::string>("enp108s0");
    float timeout     = cfg["robot"]["sdk_timeout"].as<float>(10.f);

    // 初始化 SDK（ChannelFactory 单例，只调用一次）
    unitree::robot::ChannelFactory::Instance()->Init(0, iface);

    auto sport   = std::make_shared<SportController>();
    auto monitor = std::make_shared<StateMonitor>();
    auto camera  = std::make_shared<CameraClient>();

    sport->init(timeout);
    monitor->init();
    camera->init();

    // 关闭遥控器干扰，确保程序独占控制
    sport->switchJoystick(false);

    RobotContext ctx{cfg, sport, monitor, camera};
    MissionRunner runner(std::move(ctx));
    runner.run();

    sport->switchJoystick(true);
    return 0;
}
