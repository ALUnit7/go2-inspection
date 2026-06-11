#include "StateObstacleAvoid.hpp"
#include "core/RobotContext.hpp"
#include "core/StateIDs.hpp"
#include "perception/LineDetector.hpp"
#include <unitree/robot/go2/obstacles_avoid/obstacles_avoid_client.hpp>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <cstring>
#include <iostream>
#include <unistd.h>
// ── 调参区 ───────────────────────────────────────────────────────────────────
static const int   NAV_UDP_PORT   = 9876;
static const float AVOID_SPEED    = 0.25f;
static const int   STABLE_THRESH  = 20;   // 线恢复稳定帧数
// ────────────────────────────────────────────────────────────────────────────
void StateObstacleAvoid::enter(RobotContext& ctx) {
    std::cout << "[ObstacleAvoid] enter, FreeAvoid ON\n";
    ctx.sport->freeWalk();
    // 尝试绑定 UDP 接收导航速度指令
    sock_fd_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd_ >= 0) {
        fcntl(sock_fd_, F_SETFL, O_NONBLOCK);
        sockaddr_in addr{}; addr.sin_family=AF_INET;
        addr.sin_addr.s_addr=INADDR_ANY; addr.sin_port=htons(NAV_UDP_PORT);
        if (bind(sock_fd_, (sockaddr*)&addr, sizeof(addr)) < 0) {
            close(sock_fd_); sock_fd_ = -1;
        }
    }
    stable_cnt_ = 0; nav_online_ = false;
}
int StateObstacleAvoid::update(RobotContext& ctx) {
    cv::Mat frame;
    ctx.camera->grab(frame);

    // 尝试读 UDP 导航速度指令
    float vx=AVOID_SPEED, vy=0, vyaw=0;
    if (sock_fd_ >= 0) {
        float buf[3];
        ssize_t n = recv(sock_fd_, buf, sizeof(buf), 0);
        if (n == sizeof(buf)) {
            vx=buf[0]; vy=buf[1]; vyaw=buf[2];
            nav_online_ = true;
        }
    }
    ctx.sport->move(vx, vy, vyaw);

    // 退出条件：黑线重新出现
    if (!frame.empty()) {
        LineDetector det(67,800,12,5);
        LineResult r = det.detect(frame);
        if (r.valid) stable_cnt_++;
        else stable_cnt_ = 0;
        if (stable_cnt_ > STABLE_THRESH) {
            std::cout << "[ObstacleAvoid] line restored, exit\n";
            return STATE_LINE_FOLLOW_3;
        }
    }
    return STATE_STAY;
}
void StateObstacleAvoid::exit(RobotContext& ctx) {
    ctx.sport->stop();
    if (sock_fd_ >= 0) { close(sock_fd_); sock_fd_ = -1; }
}
