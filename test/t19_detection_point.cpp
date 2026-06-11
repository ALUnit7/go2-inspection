// t19_detection_point: 检测平台完整流程
// 红色圆检测 → 停止 → 左转90° → YOLO识别 → 执行动作 → 右转90°
// 用法: sudo ./t19_detection_point <网卡名>
#include <unitree/robot/channel/channel_factory.hpp>
#include <unitree/robot/go2/sport/sport_client.hpp>
#include <unitree/robot/go2/vui/vui_client.hpp>
#include <unitree/robot/go2/video/video_client.hpp>
#include <unitree/robot/channel/channel_subscriber.hpp>
#include <unitree/idl/go2/SportModeState_.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <csignal>
#include <atomic>
#include <cmath>
#include <unistd.h>
#include "../src/perception/YoloDetector.hpp"
#include "../src/perception/YoloDetector.cpp"

// ── 调参区 ────────────────────────────────────────────────────────────────────
static const std::string MODEL_PATH = "/home/wzl/GO2_RCOM/models/warning_sign.onnx";
static const std::vector<std::string> CLASSES = {"radiation","electric","oxidizer"};
static const float CONF_THRESH    = 0.25f;
static const float TURN_SPEED     = 0.5f;   // 原地转向速度 rad/s
static const float YAW_TURN_DEG   = 85.0f;  // 转向目标角度
static const int   YOLO_TIMEOUT_S = 3;      // YOLO识别超时秒数
// 红色圆 HSV
static const int H1_LO=0,H1_HI=15, H2_LO=163,H2_HI=179;
static const int S_LO=100, V_LO=80, MIN_AREA=3000;
static const float CIRCULARITY = 0.65f;
// ─────────────────────────────────────────────────────────────────────────────

std::atomic<bool> g_running{true};
void on_sigint(int) { g_running = false; }
std::atomic<float> g_yaw{0};

bool detectRedCircle(const cv::Mat& frame) {
    cv::Mat hsv, m1, m2, mask, k;
    cv::cvtColor(frame, hsv, cv::COLOR_BGR2HSV);
    cv::inRange(hsv,cv::Scalar(H1_LO,S_LO,V_LO),cv::Scalar(H1_HI,255,255),m1);
    cv::inRange(hsv,cv::Scalar(H2_LO,S_LO,V_LO),cv::Scalar(H2_HI,255,255),m2);
    mask = m1 | m2;
    k = cv::getStructuringElement(cv::MORPH_ELLIPSE,{9,9});
    cv::morphologyEx(mask,mask,cv::MORPH_CLOSE,k);
    std::vector<std::vector<cv::Point>> cs;
    cv::findContours(mask,cs,cv::RETR_EXTERNAL,cv::CHAIN_APPROX_SIMPLE);
    for (auto& c : cs) {
        double a = cv::contourArea(c);
        if (a < MIN_AREA) continue;
        double p = cv::arcLength(c,true);
        if (4*CV_PI*a/(p*p) >= CIRCULARITY) return true;
    }
    return false;
}

int main(int argc, char** argv) {
    if (argc < 2) { fprintf(stderr,"Usage: sudo %s <iface>\n",argv[0]); return 1; }
    signal(SIGINT, on_sigint);

    unitree::robot::ChannelFactory::Instance()->Init(0, argv[1]);
    unitree::robot::ChannelSubscriber<unitree_go::msg::dds_::SportModeState_> sub("rt/sportmodestate");
    sub.InitChannel([](const void* m){
        g_yaw = ((const unitree_go::msg::dds_::SportModeState_*)m)->imu_state().rpy()[2]*180.f/M_PI;
    },1);

    unitree::robot::go2::SportClient sc;
    sc.SetTimeout(10.f); sc.Init();
    unitree::robot::go2::VuiClient vc;
    vc.SetTimeout(5.f); vc.Init();
    sleep(1);

    unitree::robot::go2::VideoClient vcam;
    vcam.SetTimeout(3.f); vcam.Init();

    YoloDetector yolo(MODEL_PATH, CLASSES, CONF_THRESH);
    sc.FreeWalk();

    enum Phase { FOLLOW, TURNING_LEFT, RECOGNIZE, ACTING, TURNING_RIGHT, DONE };
    Phase phase = FOLLOW;
    float yaw_start=0, yaw_accum=0;
    int sign_type=-1;
    auto act_start = std::chrono::steady_clock::now();

    printf("Running. Looking for red circle...\n");
    std::vector<uint8_t> buf;

    while (g_running && phase != DONE) {
        if (vcam.GetImageSample(buf)!=0) { usleep(20000); continue; }
        cv::Mat frame = cv::imdecode(cv::Mat(buf), cv::IMREAD_COLOR);
        if (frame.empty()) { usleep(20000); continue; }

        float yaw = g_yaw.load();

        switch (phase) {
        case FOLLOW:
            sc.Move(0.2f,0,0);
            if (detectRedCircle(frame)) {
                sc.StopMove(); usleep(500000);
                printf("Red circle! Turning left\n");
                yaw_start=yaw; yaw_accum=0;
                phase=TURNING_LEFT;
            }
            break;
        case TURNING_LEFT: {
            sc.Move(0,0,TURN_SPEED);
            float d=yaw-yaw_start; if(d>180)d-=360; if(d<-180)d+=360;
            yaw_accum=std::abs(d);
            if (yaw_accum>=YAW_TURN_DEG) {
                sc.StopMove(); usleep(500000);
                printf("Turned left. YOLO recognizing...\n");
                act_start=std::chrono::steady_clock::now();
                phase=RECOGNIZE;
            }
            break;
        }
        case RECOGNIZE: {
            auto dets=yolo.detect(frame);
            if (!dets.empty()) {
                sign_type=dets[0].class_id;
                printf("Sign=%d(%s) conf=%.2f\n",sign_type,dets[0].label.c_str(),dets[0].conf);
                sc.StopMove();
                phase=ACTING;
            }
            float elapsed=std::chrono::duration<float>(std::chrono::steady_clock::now()-act_start).count();
            if (elapsed>YOLO_TIMEOUT_S && sign_type<0) {
                printf("Timeout, no sign. skip.\n");
                phase=ACTING;
            }
            break;
        }
        case ACTING:
            if (sign_type==0) {
                // 辐射：灯光闪烁三次
                for(int i=0;i<3;i++){vc.SetBrightness(10);usleep(300000);vc.SetBrightness(0);usleep(300000);}
                vc.SetBrightness(10);
            } else if (sign_type==1) {
                sc.BalanceStand(); sleep(1); sc.Stretch(); sleep(4);
            } else {
                sc.BalanceStand(); sleep(1); sc.Hello(); sleep(4);
            }
            printf("Action done. Turning right\n");
            yaw_start=yaw; yaw_accum=0;
            phase=TURNING_RIGHT;
            break;
        case TURNING_RIGHT: {
            sc.Move(0,0,-TURN_SPEED);
            float d=yaw-yaw_start; if(d>180)d-=360; if(d<-180)d+=360;
            yaw_accum=std::abs(d);
            if (yaw_accum>=YAW_TURN_DEG) {
                sc.StopMove(); printf("Done.\n");
                phase=DONE;
            }
            break;
        }
        default: break;
        }

        cv::Mat vis=frame.clone();
        yolo.detect(frame,&vis);
        cv::putText(vis,"Phase:"+std::to_string(phase),{20,50},cv::FONT_HERSHEY_SIMPLEX,1,{0,255,255},2);
        cv::imshow("Detection Point",vis);
        if (cv::waitKey(1)==27) break;
        usleep(20000);
    }
    sc.StopMove(); sc.FreeWalk();
    return 0;
}
