#include "StateGrabInitial.hpp"
#include "core/RobotContext.hpp"
#include "core/StateIDs.hpp"
#include "perception/SignDetector.hpp"
#include <iostream>
#include <cmath>
static const float TURN_SPD = 0.4f;
static const float YAW_90   = 85.0f;
void StateGrabInitial::enter(RobotContext& ctx) {
    phase_=0; ticks_=0;
    ctx.sport->stop();
    std::cout << "[GrabInitial] enter\n";
}
int StateGrabInitial::update(RobotContext& ctx) {
    float yaw = ctx.monitor->rpy()[2]*180.f/M_PI;
    switch(phase_) {
    case 0: // 左转90°看标识
        ctx.sport->move(0,0,TURN_SPD);
        { float d=yaw-yaw_start_; if(d>180)d-=360;if(d<-180)d+=360;
          if(std::abs(d)>=YAW_90){ctx.sport->stop(); phase_=1; yaw_start_=yaw;} }
        break;
    case 1: { // 识别标识
        cv::Mat frame; ctx.camera->grab(frame);
        if (!frame.empty()) {
            SignDetector det;
            SignResult r = det.detect(frame);
            if (r.valid) {
                ctx.memory.marker_id = r.type;
                std::cout << "[GrabInitial] marker_id=" << r.type << "\n";
            }
        }
        if (++ticks_ > 30) { ctx.sport->stop(); phase_=2; yaw_start_=yaw; }
        break;
    }
    case 2: // 右转90°回正
        ctx.sport->move(0,0,-TURN_SPD);
        { float d=yaw-yaw_start_; if(d>180)d-=360;if(d<-180)d+=360;
          if(std::abs(d)>=YAW_90){ctx.sport->stop(); phase_=3; ticks_=0;} }
        break;
    case 3: // 左侧移到平台
        ctx.sport->move(0, 0.3f, 0);
        if (++ticks_ > 50) { ctx.sport->stop(); phase_=4; ticks_=0; }
        break;
    case 4: // 机械臂抓取（占位）
        std::cout << "[GrabInitial] ARM: grabbing initial item... (placeholder)\n";
        if (++ticks_ > 150) { phase_=5; ticks_=0; }
        break;
    case 5: // 右侧移回赛道
        ctx.sport->move(0,-0.3f,0);
        if (++ticks_ > 50) { ctx.sport->stop(); return STATE_LINE_FOLLOW_5; }
        break;
    }
    return STATE_STAY;
}
