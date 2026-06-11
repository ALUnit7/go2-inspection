// t15_obstacle_jump: 障碍物检测 + 跳跃（连机器人）
// 用法: sudo ./t15_obstacle_jump <网卡名>
// 空格：手动触发跳跃  ESC：退出
#include <unitree/robot/channel/channel_factory.hpp>
#include <unitree/robot/go2/sport/sport_client.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <csignal>
#include <atomic>
#include <chrono>
#include <unistd.h>
#include "../src/perception/HikCamera.hpp"
#include "../src/perception/HikCamera.cpp"
#include "../src/perception/ObstacleDetector.hpp"
#include "../src/perception/ObstacleDetector.cpp"

// ── 调参区 ────────────────────────────────────────────────────────────────────
static const int   THRESH      = 67;
static const int   CLOSE_K     = 12;
static const int   OPEN_K      = 5;
static const int   NEAR_ROW    = 1000;  // 触发行，越大离狗越近才触发
static const int   FAR_ROW     = 700;
static const float W_THRESH    = 0.10f; // 白色占比阈值
static const int   CONFIRM     = 3;     // 确认帧数
static const int   LAND_WAIT_MS = 2000; // 跳跃后等待落地时间(ms)
// ─────────────────────────────────────────────────────────────────────────────

std::atomic<bool> g_running{true};
void on_sigint(int) { g_running = false; }

int main(int argc, char** argv) {
    if (argc < 2) { fprintf(stderr, "Usage: sudo %s <iface>\n", argv[0]); return 1; }
    signal(SIGINT, on_sigint);

    unitree::robot::ChannelFactory::Instance()->Init(0, argv[1]);
    unitree::robot::go2::SportClient sc;
    sc.SetTimeout(10.f); sc.Init();
    sleep(1);

    HikCamera cam;
    if (!cam.init()) return 1;

    ObstacleConfig cfg;
    cfg.near_row       = NEAR_ROW;
    cfg.far_row        = FAR_ROW;
    cfg.white_thresh   = W_THRESH;
    cfg.confirm_frames = CONFIRM;
    ObstacleDetector detector(cfg);

    sc.FreeWalk();
    printf("Running. Space=manual jump, ESC=exit\n");

    bool jumping = false;
    std::chrono::steady_clock::time_point jump_time;

    while (g_running) {
        cv::Mat frame;
        if (!cam.grab(frame)) { usleep(20000); continue; }

        // 二值化
        cv::Mat gray, binary;
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
        cv::threshold(gray, binary, THRESH, 255, cv::THRESH_BINARY_INV);
        int ck = CLOSE_K, ok = OPEN_K;
        cv::Mat kc = cv::getStructuringElement(cv::MORPH_RECT, {ck,ck});
        cv::Mat ko = cv::getStructuringElement(cv::MORPH_RECT, {ok,ok});
        cv::dilate(binary, binary, kc); cv::erode(binary, binary, kc);
        cv::erode(binary, binary, ko);  cv::dilate(binary, binary, ko);

        cv::Mat vis = frame.clone();

        if (!jumping) {
            bool detected = detector.detect(binary, &vis);

            cv::putText(vis, detected ? "OBSTACLE - JUMP!" : "clear",
                        {30,50}, cv::FONT_HERSHEY_SIMPLEX, 1.5,
                        detected ? cv::Scalar{0,0,255} : cv::Scalar{0,255,0}, 3);

            if (detected) {
                printf("Obstacle detected! Jumping...\n");
                sc.StopMove();
                usleep(300000);
                sc.FrontJump();
                jumping = true;
                jump_time = std::chrono::steady_clock::now();
                detector.reset();
            }
        } else {
            // 等待落地
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - jump_time).count();
            char buf[64];
            snprintf(buf, sizeof(buf), "Landing... %ldms/%dms", elapsed, LAND_WAIT_MS);
            cv::putText(vis, buf, {30,50}, cv::FONT_HERSHEY_SIMPLEX, 1, {255,165,0}, 2);

            if (elapsed > LAND_WAIT_MS) {
                printf("Landed. Resuming FreeWalk.\n");
                sc.FreeWalk();
                jumping = false;
            }
        }

        cv::imshow("Obstacle+Jump", vis);
        char key = (char)cv::waitKey(1);
        if (key == 27) break;
        if (key == 32 && !jumping) {
            // 手动触发跳跃
            printf("Manual jump triggered.\n");
            sc.StopMove();
            usleep(300000);
            sc.FrontJump();
            jumping = true;
            jump_time = std::chrono::steady_clock::now();
            detector.reset();
        }
        usleep(20000);
    }

    sc.StopMove();
    return 0;
}
