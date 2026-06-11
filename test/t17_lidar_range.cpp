// t17_lidar_range: 雷达侧距调试（平台到位检测）
// 连机器人，实时显示前/左/右距离，记录突变时机
// 用法: sudo ./t17_lidar_range <网卡名>
#include <unitree/robot/channel/channel_factory.hpp>
#include <unitree/robot/channel/channel_subscriber.hpp>
#include <unitree/idl/ros2/PointStamped_.hpp>
#include <iostream>
#include <atomic>
#include <unistd.h>

std::atomic<float> g_front{99}, g_left{99}, g_right{99};

int main(int argc, char** argv) {
    if (argc < 2) { fprintf(stderr, "Usage: sudo %s <iface>\n", argv[0]); return 1; }

    unitree::robot::ChannelFactory::Instance()->Init(0, argv[1]);

    unitree::robot::ChannelSubscriber<geometry_msgs::msg::dds_::PointStamped_> sub("rt/utlidar/range_info");
    sub.InitChannel([](const void* msg) {
        auto& m = *(const geometry_msgs::msg::dds_::PointStamped_*)msg;
        g_front = m.point().x();
        g_left  = m.point().y();
        g_right = m.point().z();
    });

    float prev_left = 99, prev_right = 99;
    const float DELTA_THRESH = 0.3f;  // 突变阈值(m)

    printf("Listening rt/utlidar/range_info...\n");
    printf("%-10s %-10s %-10s  Event\n", "Front(m)", "Left(m)", "Right(m)");

    while (true) {
        float f = g_front, l = g_left, r = g_right;
        const char* event = "";
        if (prev_left - l > DELTA_THRESH)   event = "<-- LEFT JUMP";
        if (prev_right - r > DELTA_THRESH)  event = "--> RIGHT JUMP";
        printf("%-10.2f %-10.2f %-10.2f  %s\n", f, l, r, event);
        prev_left = l; prev_right = r;
        usleep(100000);  // 10Hz
    }
}
