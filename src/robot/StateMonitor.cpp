#include "StateMonitor.hpp"

void StateMonitor::init() {
    sub_.InitChannel([this](const void* m){ on_state(m); });
}

void StateMonitor::on_state(const void* msg) {
    std::lock_guard<std::mutex> lk(mtx_);
    latest_ = *static_cast<const SportState*>(msg);
}

std::array<float,3> StateMonitor::position() const {
    std::lock_guard<std::mutex> lk(mtx_);
    auto p = latest_.position();
    return {p[0], p[1], p[2]};
}
std::array<float,4> StateMonitor::quaternion() const {
    std::lock_guard<std::mutex> lk(mtx_);
    auto q = latest_.imu_state().quaternion();
    return {q[0], q[1], q[2], q[3]};
}
std::array<float,3> StateMonitor::rpy() const {
    std::lock_guard<std::mutex> lk(mtx_);
    auto r = latest_.imu_state().rpy();
    return {r[0], r[1], r[2]};
}
std::array<float,3> StateMonitor::velocity() const {
    std::lock_guard<std::mutex> lk(mtx_);
    auto v = latest_.velocity();
    return {v[0], v[1], v[2]};
}
int StateMonitor::errorCode() const {
    std::lock_guard<std::mutex> lk(mtx_);
    return static_cast<int>(latest_.error_code());
}
float StateMonitor::frontRange() const {
    std::lock_guard<std::mutex> lk(mtx_);
    return latest_.range_obstacle()[0];  // 前方障碍物距离
}
