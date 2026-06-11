#include "StateLineFollow.hpp"
#include "core/RobotContext.hpp"
#include "core/StateIDs.hpp"
#include "perception/LineDetector.hpp"
#include "perception/ObstacleDetector.hpp"
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <cmath>

// ── 调参区 ────────────────────────────────────────────────────────────────────
static const float FOLLOW_SPEED      = 0.20f;
static const float FOLLOW_KP         = 0.05f;
static const float FOLLOW_KP2        = 0.003f;
static const float CORNER_TURN_SPEED = 0.5f;
static const float CORNER_YAW_DEG    = 85.0f;
// 红色圆 HSV
static const int   RC_H1LO=0,  RC_H1HI=15,  RC_H2LO=163, RC_H2HI=179;
static const int   RC_SLO=100, RC_VLO=80,   RC_AREA=3000;
static const float RC_CIRC=0.65f;
// ─────────────────────────────────────────────────────────────────────────────

void StateLineFollow::enter(RobotContext& ctx) {
    speed_ = ctx.cfg["tasks"]["line_follow"]["speed"].as<float>(FOLLOW_SPEED);
    kp_    = ctx.cfg["tasks"]["line_follow"]["kp"].as<float>(FOLLOW_KP);
    kp2_   = ctx.cfg["tasks"]["line_follow"]["kp2"].as<float>(FOLLOW_KP2);
    lost_frames_ = corner_count_ = 0;
    in_corner_ = false;
    ctx.sport->freeWalk();
    std::cout << "[LineFollow] enter, next=" << cfg_.next_state << "\n";
}

bool StateLineFollow::detectObstacle(const cv::Mat& binary, int w) const {
    ObstacleDetector det;
    // 复用最后一帧的线中心（已在调用前更新到 detector.lastCx()）
    return det.detect(binary, nullptr, w / 2);
}

bool StateLineFollow::detectRedCircle(const cv::Mat& bgr) const {
    cv::Mat hsv, m1, m2, mask, k;
    cv::cvtColor(bgr, hsv, cv::COLOR_BGR2HSV);
    cv::inRange(hsv, cv::Scalar(RC_H1LO,RC_SLO,RC_VLO), cv::Scalar(RC_H1HI,255,255), m1);
    cv::inRange(hsv, cv::Scalar(RC_H2LO,RC_SLO,RC_VLO), cv::Scalar(RC_H2HI,255,255), m2);
    mask = m1 | m2;
    k = cv::getStructuringElement(cv::MORPH_ELLIPSE, {9,9});
    cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, k);
    std::vector<std::vector<cv::Point>> cs;
    cv::findContours(mask, cs, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    for (auto& c : cs) {
        double a = cv::contourArea(c);
        if (a < RC_AREA) continue;
        double p = cv::arcLength(c, true);
        if (4*M_PI*a/(p*p) >= RC_CIRC) return true;
    }
    return false;
}

int StateLineFollow::update(RobotContext& ctx) {
    cv::Mat frame;
    if (!ctx.camera->grab(frame)) return STATE_STAY;

    // 里程计累积（用速度估算，简单积分）
    ctx.memory.total_dist_m += speed_ * 0.02f;  // 50Hz

    // ── 二值化（ObstacleDetector 也需要）─────────────────────────────────
    cv::Mat gray, binary;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    cv::threshold(gray, binary, 67, 255, cv::THRESH_BINARY_INV);
    cv::Mat kc = cv::getStructuringElement(cv::MORPH_RECT, {12,12});
    cv::Mat ko = cv::getStructuringElement(cv::MORPH_RECT, {5,5});
    cv::dilate(binary, binary, kc); cv::erode(binary, binary, kc);
    cv::erode(binary, binary, ko);  cv::dilate(binary, binary, ko);

    // ── 先做线检测（获取 last_cx_ 供障碍物检测使用）────────────────────────
    LineDetector detector(67, 800, 12, 5);
    LineResult res = detector.detect(frame);

    // ── 退出条件检测 ──────────────────────────────────────────────────────
    // 障碍物检测：传入线中心位置，避免机器人偏移时 ROI 坍塌
    if (cfg_.exit_on_obstacle && detectObstacle(binary, detector.lastCx()))
        return cfg_.next_state;

    if (cfg_.exit_on_red_circle && detectRedCircle(frame))
        return cfg_.next_state;

    if (cfg_.exit_on_line_lost && !res.valid) {
        if (++lost_frames_ > 15) return cfg_.next_state;
    } else {
        lost_frames_ = 0;
    }

    // 直角弯计数
    if (cfg_.exit_on_corner) {
        float yaw = ctx.monitor->rpy()[2] * 180.f / M_PI;
        if (!in_corner_ && res.valid && !res.top_ok) {
            in_corner_  = true;
            yaw_start_  = yaw;
            yaw_accum_  = 0;
        }
        if (in_corner_) {
            ctx.sport->move(speed_ * 0.3f, 0, CORNER_TURN_SPEED);
            float d = yaw - yaw_start_;
            if (d > 180) d -= 360; if (d < -180) d += 360;
            yaw_accum_ = std::abs(d);
            if (yaw_accum_ >= CORNER_YAW_DEG) {
                corner_count_++;
                ctx.memory.corner_count = corner_count_;
                in_corner_ = false;
                ctx.sport->freeWalk();
                if (corner_count_ >= cfg_.target_corners)
                    return cfg_.next_state;
            }
            return STATE_STAY;
        }
    }

    if (cfg_.exit_on_odometry) {
        // TODO: 里程计+雷达侧距，当前先用固定距离占位
        // if (ctx.memory.total_dist_m > cfg_.target_distance) return cfg_.next_state;
    }

    // ── 运动控制（正常寻迹）──────────────────────────────────────────────
    if (!res.valid) {
        if (++lost_frames_ > 30) ctx.sport->stop();
        return STATE_STAY;
    }
    lost_frames_ = 0;

    if (res.top_ok) {
        float vyaw = -(float)(res.angle * kp_);
        float vy   =  (float)(res.offset * kp2_);
        ctx.sport->move(speed_, vy, vyaw);
    } else {
        // 弯道：减速惯性
        ctx.sport->move(speed_ * 0.3f, 0, 0);
    }
    return STATE_STAY;
}

void StateLineFollow::exit(RobotContext& ctx) {
    ctx.sport->stop();
}
