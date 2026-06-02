#pragma once
#include <unitree/idl/go2/SportModeState_.hpp>
#include <unitree/robot/channel/channel_subscriber.hpp>
#include <mutex>
#include <array>

// 订阅 rt/sportmodestate，线程安全地提供最新状态快照
class StateMonitor {
public:
    void init();

    std::array<float,3> position() const;
    std::array<float,4> quaternion() const;
    std::array<float,3> rpy() const;
    std::array<float,3> velocity() const;
    int   errorCode() const;
    int   error_code() const { return errorCode(); }  // 兼容旧调用
    float frontRange() const;  // 前方障碍物距离 m（来自 rangeObstacle()[0]）

private:
    using SportState = unitree_go::msg::dds_::SportModeState_;
    void on_state(const void* msg);

    mutable std::mutex mtx_;
    SportState latest_{};
    unitree::robot::ChannelSubscriber<SportState> sub_{"rt/sportmodestate"};
};
