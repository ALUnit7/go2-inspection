// t1_connection: 验证 SDK 连接，持续打印 IMU/位置/速度/error_code
// 用法: sudo ./t1_connection <网卡名>
#include <unitree/robot/channel/channel_factory.hpp>
#include <unitree/robot/channel/channel_subscriber.hpp>
#include <unitree/idl/go2/SportModeState_.hpp>
#include <iostream>
#include <unistd.h>

using State = unitree_go::msg::dds_::SportModeState_;

int main(int argc, char** argv) {
    if (argc < 2) { std::cerr << "Usage: " << argv[0] << " <iface>\n"; return 1; }

    unitree::robot::ChannelFactory::Instance()->Init(0, argv[1]);

    unitree::robot::ChannelSubscriber<State> sub("rt/sportmodestate");
    sub.InitChannel([](const void* msg) {
        auto& s = *(const State*)msg;
        printf("pos=(%.2f,%.2f,%.2f) rpy=(%.2f,%.2f,%.2f) vel=(%.2f,%.2f,%.2f) mode=%u\n",
            s.position()[0], s.position()[1], s.position()[2],
            s.imu_state().rpy()[0], s.imu_state().rpy()[1], s.imu_state().rpy()[2],
            s.velocity()[0], s.velocity()[1], s.velocity()[2],
            s.error_code());
    }, 1);

    std::cout << "Listening rt/sportmodestate ... Ctrl+C to stop\n";
    while (1) sleep(1);
}
