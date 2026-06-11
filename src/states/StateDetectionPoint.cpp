#include "StateDetectionPoint.hpp"
#include "core/RobotContext.hpp"
#include "core/StateIDs.hpp"
#include <unitree/robot/go2/vui/vui_client.hpp>
#include <iostream>
#include <cmath>
#include <unistd.h>
static const float TURN_SPD = 0.5f;
static const float YAW_90   = 85.0f;
void StateDetectionPoint::enter(RobotContext& ctx) {
    phase_=TURN_L; ticks_=0;
    ctx.sport->stop();
    yaw_start_ = ctx.monitor->rpy()[2]*180.f/M_PI;
    std::cout << "[DetectionPoint] enter, turning left\n";
}
int StateDetectionPoint::update(RobotContext& ctx) {
    float yaw = ctx.monitor->rpy()[2]*180.f/M_PI;
    switch(phase_) {
    case TURN_L:
        ctx.sport->move(0,0,TURN_SPD);
        { float d=yaw-yaw_start_; if(d>180)d-=360;if(d<-180)d+=360;
          yaw_accum_=std::abs(d);
          if(yaw_accum_>=YAW_90){ctx.sport->stop(); phase_=RECOGNIZE; ticks_=0;} }
        break;
    case RECOGNIZE: {
        // 用 Detector 识别（简化：从 ctx 配置获取类型）
        // 当前占位：直接用配置里预设的 warning_sign
        int sign = ctx.cfg["tasks"]["detection_point"]["default_sign"].as<int>(0);
        ctx.memory.warning_sign = sign;
        std::cout << "[DetectionPoint] sign=" << sign << "\n";
        phase_=ACT; ticks_=0;
        break;
    }
    case ACT:
        if (ctx.memory.warning_sign == 0) {
            // 当心辐射：灯光闪烁三次
            unitree::robot::go2::VuiClient vc;
            vc.SetTimeout(3.f); vc.Init();
            for(int i=0;i<3;i++){vc.SetBrightness(10);usleep(300000);vc.SetBrightness(0);usleep(300000);}
            vc.SetBrightness(10);
        } else if (ctx.memory.warning_sign == 1) {
            ctx.sport->balanceStand(); sleep(1); ctx.sport->stretch(); sleep(4);
        } else {
            ctx.sport->balanceStand(); sleep(1); ctx.sport->hello(); sleep(4);
        }
        phase_=TURN_R; yaw_start_=ctx.monitor->rpy()[2]*180.f/M_PI; yaw_accum_=0;
        std::cout << "[DetectionPoint] action done, turning right\n";
        break;
    case TURN_R:
        ctx.sport->move(0,0,-TURN_SPD);
        { float d=yaw-yaw_start_; if(d>180)d-=360;if(d<-180)d+=360;
          yaw_accum_=std::abs(d);
          if(yaw_accum_>=YAW_90){ctx.sport->stop(); phase_=DONE;} }
        break;
    case DONE:
        ctx.sport->freeWalk();
        return STATE_LINE_FOLLOW_7;
    }
    return STATE_STAY;
}
