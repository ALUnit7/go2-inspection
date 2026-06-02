// t3_jump: BalanceStand -> FrontJump -> 等待落地
// 用法: sudo ./t3_jump <网卡名>
// 警告: 确保机器人前方有足够空间（至少1m）
#include <unitree/robot/channel/channel_factory.hpp>
#include <unitree/robot/go2/sport/sport_client.hpp>
#include <unitree/robot/channel/channel_subscriber.hpp>
#include <unitree/idl/go2/SportModeState_.hpp>
#include <iostream>
#include <atomic>
#include <unistd.h>

std::atomic<uint32_t> g_mode{0};

int main(int argc, char** argv) {
    if (argc < 2) { std::cerr << "Usage: " << argv[0] << " <iface>\n"; return 1; }

    unitree::robot::ChannelFactory::Instance()->Init(0, argv[1]);

    // 订阅状态，监控 error_code 判断跳跃完成
    unitree::robot::ChannelSubscriber<unitree_go::msg::dds_::SportModeState_> sub("rt/sportmodestate");
    sub.InitChannel([](const void* m){
        g_mode = ((const unitree_go::msg::dds_::SportModeState_*)m)->error_code();
    }, 1);

    unitree::robot::go2::SportClient sc;
    sc.SetTimeout(10.f); sc.Init();
    sleep(1);

    std::cout << "BalanceStand...\n"; sc.BalanceStand(); sleep(2);
    std::cout << "FrontJump! mode=" << g_mode.load() << "\n"; sc.FrontJump();

    // 等待落地：error_code 回到非跳跃状态（1008=前跳）
    int wait = 0;
    while (g_mode.load() == 1008 && wait++ < 50) usleep(100000);
    std::cout << "Landed. mode=" << g_mode.load() << "\n";

    sleep(1); sc.FreeWalk();
    return 0;
}
