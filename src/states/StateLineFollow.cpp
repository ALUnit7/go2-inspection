#include "StateLineFollow.hpp"
#include "core/RobotContext.hpp"
#include "core/StateIDs.hpp"
#include <opencv2/imgproc.hpp>
#include <iostream>

void StateLineFollow::enter(RobotContext& ctx) {
    std::cout << "[LineFollow] enter\n";
    speed_ = ctx.cfg["tasks"]["line_follow"]["speed"].as<float>(0.3f);
    kp_    = ctx.cfg["tasks"]["line_follow"]["kp"].as<float>(0.005f);
    frames_lost_ = 0;
    ctx.sport->freeWalk();
}

bool StateLineFollow::compute_error(const cv::Mat& frame, float& error) {
    // 取图像下 1/3 区域，转灰度，二值化，找最大轮廓重心
    cv::Mat roi = frame(cv::Rect(0, frame.rows*2/3, frame.cols, frame.rows/3));
    cv::Mat gray, bin;
    cv::cvtColor(roi, gray, cv::COLOR_BGR2GRAY);
    cv::threshold(gray, bin, 60, 255, cv::THRESH_BINARY_INV); // 深色线

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(bin, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    if (contours.empty()) return false;

    auto& c = *std::max_element(contours.begin(), contours.end(),
        [](auto& a, auto& b){ return cv::contourArea(a) < cv::contourArea(b); });
    if (cv::contourArea(c) < 500) return false;

    auto M = cv::moments(c);
    float cx = (float)(M.m10 / M.m00);
    error = cx - frame.cols / 2.0f;
    return true;
}

int StateLineFollow::update(RobotContext& ctx) {
    cv::Mat frame;
    if (!ctx.camera->grab(frame)) return STATE_STAY;

    float error = 0;
    if (!compute_error(frame, error)) {
        if (++frames_lost_ > 30) {
            std::cout << "[LineFollow] line lost, stopping\n";
            ctx.sport->stop();
            return STATE_STAY; // TODO: 根据比赛流程决定下一状态
        }
        return STATE_STAY;
    }
    frames_lost_ = 0;

    float vyaw = -kp_ * error;
    ctx.sport->move(speed_, 0, vyaw);
    return STATE_STAY;
    // TODO: 检测到障碍物标记时 return STATE_JUMP / STATE_OBSTACLE_AVOID
}

void StateLineFollow::exit(RobotContext& ctx) {
    ctx.sport->stop();
}
