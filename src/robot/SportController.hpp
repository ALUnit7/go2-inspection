#pragma once
#include <unitree/robot/go2/sport/sport_client.hpp>

// SportClient 的薄封装，统一错误日志，提供安全停止
class SportController {
public:
    void init(float timeout = 10.f);  // ChannelFactory 由 main 统一初始化

    int move(float vx, float vy, float vyaw);
    int stop();
    int balanceStand();
    int recoveryStand();
    int freeWalk();
    int walkStair(bool enable);  // 爬楼梯模式
    int jump();          // FrontJump
    int hello();         // 打招呼（当心强氧化物）
    int stretch();       // 伸懒腰（当心触电）
    int sit();
    int riseSit();
    int switchJoystick(bool enable);

private:
    unitree::robot::go2::SportClient client_;
    bool log_error(const char* fn, int code);
};
