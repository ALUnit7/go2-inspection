// t6_actions: 验证检测平台三个警示动作
//   当心触电     -> Stretch (伸懒腰)
//   当心强氧化物 -> Hello   (打招呼)
//   当心辐射     -> 灯光闪烁三次 (见 t7_vui_light)
// 用法: sudo ./t6_actions <网卡名> <0|1|2>
#include <unitree/robot/channel/channel_factory.hpp>
#include <unitree/robot/go2/sport/sport_client.hpp>
#include <iostream>
#include <unistd.h>

int main(int argc, char** argv) {
    if (argc < 3) { std::cerr << "Usage: " << argv[0] << " <iface> <0=触电|1=强氧化物|2=辐射>\n"; return 1; }

    unitree::robot::ChannelFactory::Instance()->Init(0, argv[1]);
    unitree::robot::go2::SportClient sc;
    sc.SetTimeout(10.f); sc.Init();
    sleep(1);

    int sign = std::stoi(argv[2]);
    switch (sign) {
    case 0:
        std::cout << "当心触电 -> Stretch\n";
        sc.BalanceStand(); sleep(1);
        sc.Stretch();      sleep(4);
        break;
    case 1:
        std::cout << "当心强氧化物 -> Hello\n";
        sc.BalanceStand(); sleep(1);
        sc.Hello();        sleep(4);
        break;
    case 2:
        std::cout << "当心辐射 -> 灯光闪烁 (见 t7_vui_light)\n";
        break;
    default:
        std::cerr << "unknown sign\n"; return 1;
    }
    sc.FreeWalk();
    return 0;
}
