// t4_obstacle_avoid: 开启 FreeAvoid -> 前进5s -> 关闭
// 用法: sudo ./t4_obstacle_avoid <网卡名>
#include <unitree/robot/channel/channel_factory.hpp>
#include <unitree/robot/go2/sport/sport_client.hpp>
#include <unitree/robot/go2/obstacles_avoid/obstacles_avoid_client.hpp>
#include <iostream>
#include <unistd.h>

int main(int argc, char** argv) {
    if (argc < 2) { std::cerr << "Usage: " << argv[0] << " <iface>\n"; return 1; }

    unitree::robot::ChannelFactory::Instance()->Init(0, argv[1]);

    unitree::robot::go2::SportClient sc;
    sc.SetTimeout(10.f); sc.Init();

    unitree::robot::go2::ObstaclesAvoidClient oc;
    oc.SetTimeout(10.f); oc.Init();

    sleep(1);

    // 方式1: SDK FreeAvoid 步态（内置避障）
    std::cout << "FreeAvoid ON\n"; sc.FreeAvoid(true); sleep(1);
    std::cout << "Move fwd...\n";  sc.Move(0.3, 0, 0); sleep(5);
    std::cout << "Stop\n";         sc.StopMove();
    std::cout << "FreeAvoid OFF\n"; sc.FreeAvoid(false);

    sleep(1);

    // 方式2: ObstaclesAvoidClient（需先 UseRemoteCommandFromApi）
    std::cout << "\nObstaclesAvoidClient: SwitchSet ON\n";
    oc.SwitchSet(true);
    oc.UseRemoteCommandFromApi(true);
    std::cout << "Move via ObstaclesAvoidClient...\n";
    oc.Move(0.3, 0, 0); sleep(3);
    oc.Move(0, 0, 0);
    oc.SwitchSet(false);

    sc.FreeWalk();
    return 0;
}
