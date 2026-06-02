// t2_basic_motion: FreeWalk 步态下前进2s -> 停止
// 用法: sudo ./t2_basic_motion <网卡名>
#include <unitree/robot/channel/channel_factory.hpp>
#include <unitree/robot/go2/sport/sport_client.hpp>
#include <iostream>
#include <unistd.h>

int main(int argc, char** argv) {
    if (argc < 2) { std::cerr << "Usage: " << argv[0] << " <iface>\n"; return 1; }

    unitree::robot::ChannelFactory::Instance()->Init(0, argv[1]);
    unitree::robot::go2::SportClient sc;
    sc.SetTimeout(10.f); sc.Init();

    sleep(1);
    std::cout << "FreeWalk...\n";  sc.FreeWalk();   sleep(1);
    std::cout << "Move fwd 0.3...\n"; sc.Move(0.3, 0, 0); sleep(2);
    std::cout << "Stop\n";         sc.StopMove();   sleep(1);
    std::cout << "Move back...\n"; sc.Move(-0.3, 0, 0); sleep(2);
    std::cout << "Stop\n";         sc.StopMove();
    return 0;
}
