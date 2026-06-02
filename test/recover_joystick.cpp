// recover_joystick: 恢复遥控器控制
// 用法: sudo ./recover_joystick <网卡名>
#include <unitree/robot/channel/channel_factory.hpp>
#include <unitree/robot/go2/sport/sport_client.hpp>
#include <iostream>
#include <unistd.h>

int main(int argc, char** argv) {
    if (argc < 2) { std::cerr << "Usage: sudo " << argv[0] << " <iface>\n"; return 1; }

    unitree::robot::ChannelFactory::Instance()->Init(0, argv[1]);
    unitree::robot::go2::SportClient sc;
    sc.SetTimeout(5.f); sc.Init();
    sleep(1);

    sc.StopMove();
    int ret = sc.SwitchJoystick(true);
    if (ret == 0)
        std::cout << "遥控器已恢复\n";
    else
        std::cerr << "失败，错误码: " << ret << "\n";
    return ret;
}
