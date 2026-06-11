// t20_corner_count: 直角弯检测+计数调试
// 连机器人，沿赛道行走，统计直角弯次数
// top_ok=false 时切纯yaw控制转弯，yaw累计>85°算完成一次
// 用法: sudo ./t20_corner_count <网卡名>
#include <unitree/robot/channel/channel_factory.hpp>
#include <unitree/robot/go2/sport/sport_client.hpp>
#include <unitree/robot/channel/channel_subscriber.hpp>
#include <unitree/idl/go2/SportModeState_.hpp>
#include <opencv2/highgui.hpp>
#include <csignal>
#include <atomic>
#include <cmath>
#include <unistd.h>
#include "../src/perception/HikCamera.hpp"
#include "../src/perception/HikCamera.cpp"
#include "../src/perception/LineDetector.hpp"
#include "../src/perception/LineDetector.cpp"

// ── 调参区 ────────────────────────────────────────────────────────────────────
static const int   THRESH      = 67;
static const int   CLOSE_K     = 12;
static const int   OPEN_K      = 5;
static const float SPEED       = 0.20f;
static const float KP          = 0.05f;
static const float KP2         = 0.003f;
static const float TURN_SPEED  = 0.5f;   // 直角弯转向速度 rad/s
static const float YAW_TURN_DEG= 85.0f;  // 转过此角度认为弯道完成
// ─────────────────────────────────────────────────────────────────────────────

std::atomic<bool> g_running{true};
void on_sigint(int) { g_running = false; }
std::atomic<float> g_yaw{0};

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
    sleep(1);

    HikCamera cam;
    if (!cam.init()) return 1;

    LineDetector detector(THRESH, 800, CLOSE_K, OPEN_K);
    sc.FreeWalk();

    int corner_count = 0;
    bool in_corner   = false;
    float yaw_start  = 0, yaw_accum = 0;
    int lost = 0;

    printf("Running. Corner count will print on each detection.\n");

    while (g_running) {
        cv::Mat frame;
        if (!cam.grab(frame)) { usleep(20000); continue; }

        LineResult res = detector.detectAndDraw(frame);
        float yaw = g_yaw.load();

        if (!in_corner) {
            if (res.valid && !res.top_ok) {
                // 直角弯开始：top_ok=false
                in_corner  = true;
                yaw_start  = yaw;
                yaw_accum  = 0;
                printf("Corner start! count so far=%d\n", corner_count);
            }
            if (res.valid) {
                float vyaw = -(float)(res.angle * KP);
                float vy   =  (float)(res.offset * KP2);
                sc.Move(SPEED, vy, vyaw);
            } else {
                if (++lost > 30) sc.StopMove();
            }
        } else {
            // 纯 yaw 转弯
            sc.Move(SPEED * 0.3f, 0, TURN_SPEED);
            float d = yaw - yaw_start;
            if (d >  180) d -= 360;
            if (d < -180) d += 360;
            yaw_accum = std::abs(d);
            if (yaw_accum >= YAW_TURN_DEG) {
                corner_count++;
                in_corner = false;
                printf("Corner complete! total corners=%d\n", corner_count);
                sc.FreeWalk();
            }
        }

        // 叠加信息
        char buf[64];
        snprintf(buf, sizeof(buf), "Corners:%d %s yaw_acc=%.0f",
                 corner_count, in_corner?"TURNING":"follow", yaw_accum);
        cv::putText(frame, buf, {20,50}, cv::FONT_HERSHEY_SIMPLEX, 1,
                    in_corner ? cv::Scalar{0,0,255} : cv::Scalar{0,255,0}, 2);
        cv::imshow("Corner Count", frame);
        if (cv::waitKey(1) == 27) break;
        usleep(20000);
    }
    sc.StopMove();
    return 0;
}
