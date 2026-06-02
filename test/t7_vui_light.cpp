// t7_vui_light: 前灯闪烁三次，对应"当心辐射"警示动作
// 用法: sudo ./t7_vui_light <网卡名>
#include <unitree/robot/channel/channel_factory.hpp>
#include <unitree/robot/go2/vui/vui_client.hpp>
#include <iostream>
#include <unistd.h>

int main(int argc, char** argv) {
    if (argc < 2) { std::cerr << "Usage: " << argv[0] << " <iface>\n"; return 1; }

    unitree::robot::ChannelFactory::Instance()->Init(0, argv[1]);
    unitree::robot::go2::VuiClient vc;
    vc.SetTimeout(5.f); vc.Init();
    sleep(1);

    std::cout << "Flash light x3\n";
    for (int i = 0; i < 3; i++) {
        vc.SetBrightness(10); usleep(300000);  // 亮 0.3s
        vc.SetBrightness(0);  usleep(300000);  // 灭 0.3s
    }
    vc.SetBrightness(10);  // 恢复常亮
    std::cout << "Done\n";
    return 0;
}
