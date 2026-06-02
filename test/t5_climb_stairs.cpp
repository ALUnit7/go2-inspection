// t5_climb_stairs: FreeWalk 低速前进，订阅 pitch 角监控上下台阶
// 用法: sudo ./t5_climb_stairs <网卡名>
#include <unitree/robot/channel/channel_factory.hpp>
#include <unitree/robot/go2/sport/sport_client.hpp>
#include <unitree/robot/channel/channel_subscriber.hpp>
#include <unitree/idl/go2/SportModeState_.hpp>
#include <iostream>
#include <atomic>
#include <unistd.h>

std::atomic<float> g_pitch{0};
std::atomic<float> g_vx{0};

int main(int argc, char** argv) {
    if (argc < 2) { std::cerr << "Usage: " << argv[0] << " <iface>\n"; return 1; }

    unitree::robot::ChannelFactory::Instance()->Init(0, argv[1]);

    unitree::robot::ChannelSubscriber<unitree_go::msg::dds_::SportModeState_> sub("rt/sportmodestate");
    sub.InitChannel([](const void* m){
        auto& s = *(const unitree_go::msg::dds_::SportModeState_*)m;
        g_pitch = s.imu_state().rpy()[1];
        g_vx    = s.velocity()[0];
    }, 1);

    unitree::robot::go2::SportClient sc;
    sc.SetTimeout(10.f); sc.Init();
    sleep(1);

    std::cout << "FreeWalk + slow forward. Watch pitch angle.\n";
    sc.FreeWalk();
    sleep(1);

    // 低速前进，持续打印 pitch
    for (int i = 0; i < 100; i++) {  // 10s
        sc.Move(0.2f, 0, 0);
        printf("pitch=%.3f rad  vx=%.3f m/s\n", (float)g_pitch, (float)g_vx);
        usleep(100000);
    }
    sc.StopMove();
    return 0;
}
