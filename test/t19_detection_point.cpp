// t19_detection_point: 检测平台完整流程
// 海康相机：检测地面红色圆（寻迹中触发）
// GO2前置摄像头：YOLO识别警示标志（转身后拍）
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
#include "../src/perception/HikCamera.hpp"
#include "../src/perception/HikCamera.cpp"
#include "../src/perception/YoloDetector.hpp"
#include "../src/perception/YoloDetector.cpp"

// ── 调参区 ────────────────────────────────────────────────────────────────────
static const std::string MODEL_PATH = "/home/wzl/GO2_RCOM/models/warning_sign.onnx";
static const std::vector<std::string> CLASSES = {"radiation","electric","oxidizer"};
static const float CONF_THRESH    = 0.25f;
static const float TURN_SPEED     = 0.5f;
static const float YAW_TURN_DEG   = 85.0f;
static const int   YOLO_TIMEOUT_S = 3;
// 红色圆 HSV（海康相机，与 t16 一致）
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
    cv::inRange(hsv, cv::Scalar(H1_LO,S_LO,V_LO), cv::Scalar(H1_HI,255,255), m1);
    cv::inRange(hsv, cv::Scalar(H2_LO,S_LO,V_LO), cv::Scalar(H2_HI,255,255), m2);
    mask = m1 | m2;
    k = cv::getStructuringElement(cv::MORPH_ELLIPSE, {9,9});
    cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, k);
    std::vector<std::vector<cv::Point>> cs;
    cv::findContours(mask, cs, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    for (auto& c : cs) {
        double a = cv::contourArea(c);
        if (a < MIN_AREA) continue;
        double p = cv::arcLength(c, true);
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
    unitree::robot::go2::VuiClient vc_light;
    vc_light.SetTimeout(5.f); vc_light.Init();

    // 海康相机：检测地面红色圆
    HikCamera hik;
    if (!hik.init()) return 1;

    // GO2 前置摄像头：YOLO 识别警示标志
    unitree::robot::go2::VideoClient vcam;
    vcam.SetTimeout(3.f); vcam.Init();

    YoloDetector yolo(MODEL_PATH, CLASSES, CONF_THRESH);
    sc.FreeWalk();

    enum Phase { FOLLOW, TURNING_LEFT, RECOGNIZE, ACTING, TURNING_RIGHT, DONE };
    Phase phase = FOLLOW;
    float yaw_start=0, yaw_accum=0;
    int sign_type=-1, recognize_ticks=0;

    printf("Running. Hik=red circle, GO2cam=YOLO. Ctrl+C stop\n");
    std::vector<uint8_t> buf;

    while (g_running && phase != DONE) {
        float yaw = g_yaw.load();

        // 海康相机取图（寻迹+红色圆检测）
        cv::Mat hik_frame;
        bool hik_ok = hik.grab(hik_frame);

        switch (phase) {
        case FOLLOW:
            sc.Move(0.2f, 0, 0);
            if (hik_ok && detectRedCircle(hik_frame)) {
                sc.StopMove(); usleep(500000);
                printf("Red circle detected! Turning left\n");
                yaw_start=yaw; yaw_accum=0;
                phase=TURNING_LEFT;
            }
            break;

        case TURNING_LEFT: {
            sc.Move(0, 0, TURN_SPEED);
            float d=yaw-yaw_start; if(d>180)d-=360; if(d<-180)d+=360;
            if (std::abs(d) >= YAW_TURN_DEG) {
                sc.StopMove(); usleep(500000);
                printf("Turned left. YOLO recognizing...\n");
                recognize_ticks=0; sign_type=-1;
                phase=RECOGNIZE;
            }
            break;
        }

        case RECOGNIZE: {
            // GO2 前置摄像头取图做 YOLO 推理
            if (vcam.GetImageSample(buf) == 0) {
                cv::Mat go2_frame = cv::imdecode(cv::Mat(buf), cv::IMREAD_COLOR);
                if (!go2_frame.empty()) {
                    auto dets = yolo.detect(go2_frame);
                    if (!dets.empty()) {
                        sign_type = dets[0].class_id;
                        printf("Sign=%d(%s) conf=%.2f\n",
                               sign_type, dets[0].label.c_str(), dets[0].conf);
                        phase=ACTING; break;
                    }
                }
            }
            if (++recognize_ticks > YOLO_TIMEOUT_S*50) {
                printf("Timeout, no sign detected\n");
                phase=ACTING;
            }
            break;
        }

        case ACTING:
            if (sign_type == 0) {
                // 当心辐射：灯光闪烁三次
                for (int i=0;i<3;i++) {
                    vc_light.SetBrightness(10); usleep(300000);
                    vc_light.SetBrightness(0);  usleep(300000);
                }
                vc_light.SetBrightness(10);
            } else if (sign_type == 1) {
                sc.BalanceStand(); sleep(1); sc.Stretch(); sleep(4);
            } else {
                sc.BalanceStand(); sleep(1); sc.Hello(); sleep(4);
            }
            printf("Action done. Turning right\n");
            yaw_start=yaw; yaw_accum=0;
            phase=TURNING_RIGHT;
            break;

        case TURNING_RIGHT: {
            sc.Move(0, 0, -TURN_SPEED);
            float d=yaw-yaw_start; if(d>180)d-=360; if(d<-180)d+=360;
            if (std::abs(d) >= YAW_TURN_DEG) {
                sc.StopMove(); printf("Done.\n");
                phase=DONE;
            }
            break;
        }
        default: break;
        }

        // 显示海康相机画面
        if (hik_ok) {
            cv::putText(hik_frame, "Phase:"+std::to_string(phase),
                        {20,50}, cv::FONT_HERSHEY_SIMPLEX, 1, {0,255,255}, 2);
            cv::imshow("Hik(red circle)", hik_frame);
        }
        if (cv::waitKey(1) == 27) break;
        usleep(20000);
    }

    sc.StopMove(); sc.FreeWalk();
    return 0;
}
