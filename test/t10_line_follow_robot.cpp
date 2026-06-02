// t10_line_follow_robot: 寻迹 + 机器人运动控制
// 用法: sudo ./t10_line_follow_robot <网卡名>
// 日志保存到 /tmp/line_follow_<时间戳>.csv
#include <unitree/robot/channel/channel_factory.hpp>
#include <unitree/robot/go2/sport/sport_client.hpp>
#include <opencv2/highgui.hpp>
#include <csignal>
#include <atomic>
#include <fstream>
#include <chrono>
#include <unistd.h>
#include "../src/perception/LineDetector.hpp"
#include "../src/perception/LineDetector.cpp"
#include "../src/perception/HikCamera.hpp"
#include "../src/perception/HikCamera.cpp"

// ── 调参区 ────────────────────────────────────────────────────────────────────
static const int   THRESH      = 67;
static const int   CLOSE_K     = 12;  // 闭运算核（填充白色内黑色空洞）
static const int   OPEN_K      = 5;   // 开运算核（去除外部噪点）
static const float KP          = 0.05f;
static const float KP2         = 0.001f;
static const float SPEED       = 0.21f;
static const int   CROSS_WIDTH  = 300;  // 线宽超过此值判断为十字路口（像素）
static const float EXPOSURE_US = -1;
static const float GAIN        = -1;
// ─────────────────────────────────────────────────────────────────────────────

std::atomic<bool> g_running{true};
void on_sigint(int) { g_running = false; }

int main(int argc, char** argv) {
    if (argc < 2) { fprintf(stderr, "Usage: sudo %s <iface>\n", argv[0]); return 1; }
    signal(SIGINT, on_sigint);

    // 日志文件：/tmp/line_follow_<时间戳>.csv
    auto now = std::chrono::system_clock::now();
    auto ts  = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
    std::string log_path = "/tmp/line_follow_" + std::to_string(ts) + ".csv";
    std::ofstream log(log_path);
    log << "time_ms,angle,offset,vy,vyaw,valid,top_ok\n";
    auto t0 = std::chrono::steady_clock::now();
    printf("Log: %s\n", log_path.c_str());

    unitree::robot::ChannelFactory::Instance()->Init(0, argv[1]);
    unitree::robot::go2::SportClient sc;
    sc.SetTimeout(10.f); sc.Init();
    sleep(1);

    HikConfig cfg{ EXPOSURE_US, GAIN };
    HikCamera cam;
    if (!cam.init(cfg)) return 1;

    LineDetector detector(THRESH, 800, CLOSE_K, OPEN_K);
    sc.FreeWalk();

    int   lost      = 0;
    float last_vyaw = 0;
    while (g_running) {
        cv::Mat frame;
        if (!cam.grab(frame)) { usleep(20000); continue; }

        LineResult res = detector.detectAndDraw(frame);

        float ms = std::chrono::duration<float, std::milli>(
            std::chrono::steady_clock::now() - t0).count();

        float vyaw = 0, vy = 0;
        if (res.valid) {
            lost = 0;
            if (res.line_width > CROSS_WIDTH) {
                // 十字路口：线宽异常，直行忽略控制量
                sc.Move(SPEED, 0, 0);
                printf("angle=%6.2f  offset=%6.1f  width=%d  [CROSS]\n",
                       res.angle, res.offset, res.line_width);
            } else {
                vyaw = -(float)(res.angle  * KP);
                vy   =  (float)(res.offset * KP2);
                if (res.top_ok) {
                    sc.Move(SPEED, vy, vyaw);
                    printf("angle=%6.2f  offset=%6.1f  vy=%6.3f  vyaw=%6.3f  [OK]\n",
                           res.angle, res.offset, vy, vyaw);
                } else {
                    sc.Move(SPEED * 0.3f, 0, last_vyaw);
                    vyaw = last_vyaw;
                    printf("angle=%6.2f  offset=%6.1f  vy=%6.3f  vyaw=%6.3f  [TURN]\n",
                           res.angle, res.offset, vy, vyaw);
                }
                last_vyaw = vyaw;
            }
        } else {
            if (++lost > 30) { sc.StopMove(); printf("Line lost!\n"); }
        }

        // 写日志（每帧）
        log << ms << ","
            << res.angle  << ","
            << res.offset << ","
            << vy         << ","
            << vyaw       << ","
            << res.valid  << ","
            << res.top_ok << "\n";

        cv::imshow("LineFollow", frame);
        if (cv::waitKey(1) == 27) break;
        usleep(20000);
    }

    sc.StopMove();
    log.close();
    printf("Log saved: %s\n", log_path.c_str());
    return 0;
}
