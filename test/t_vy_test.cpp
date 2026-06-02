// t_vy_test: 测试 vy 正方向
// 机器人先向 vy=+0.3 移动2s，再向 vy=-0.3 移动2s
// 观察哪个方向是左，哪个是右
// 用法: sudo ./t_vy_test <网卡名>
#include <unitree/robot/channel/channel_factory.hpp>
#include <unitree/robot/go2/sport/sport_client.hpp>
#include <unistd.h>
#include <iostream>

int main(int argc, char** argv) {
    if (argc < 2) { std::cerr << "Usage: sudo " << argv[0] << " <iface>\n"; return 1; }

    unitree::robot::ChannelFactory::Instance()->Init(0, argv[1]);
    unitree::robot::go2::SportClient sc;
    sc.SetTimeout(10.f); sc.Init();
    sleep(1);

    sc.BalanceStand();
    sleep(1);

    std::cout << "vy=+0.3 移动2s（观察方向）...\n";
    for (int i = 0; i < 100; i++) { sc.Move(0, 0.3f, 0); usleep(20000); }
    sc.StopMove(); sleep(1);

    std::cout << "vy=-0.3 移动2s（观察方向）...\n";
    for (int i = 0; i < 100; i++) { sc.Move(0, -0.3f, 0); usleep(20000); }
    sc.StopMove();

    std::cout << "完成。记录 vy+ 是左还是右。\n";
    return 0;
}
