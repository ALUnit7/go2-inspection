// t11_aruco_tag: ArUco Tag 检测测试（GO2 前置摄像头）
// 用法: sudo ./t11_aruco_tag <网卡名>
//
// ── 使用步骤 ──────────────────────────────────────────────────────────────────
// Step 1: 确认 TAG_SIZE
//   比赛规则：ArucoTag 10×10cm（含白边），黑色区域约 7cm
//   用尺子实测 Tag 黑色区域边长，填入 TAG_SIZE（单位：m）
//
// Step 2: 标定 FOCAL_LENGTH
//   将 Tag 放在已知距离 D（如 1.0m）正前方，运行程序
//   终端打印：ID=0  dist=xxx  pixelLen=42.3px
//   计算：FOCAL_LENGTH = pixelLen × D / TAG_SIZE = 42.3 × 1.0 / 0.07 ≈ 604
//   将计算值填入调参区，重新编译，验证 dist 是否准确
//
// Step 3: 设置 TARGET_TAG_ID
//   确认台阶上贴的 Tag ID（通过本程序观察），填入 TARGET_TAG_ID
//   程序会在距离 < TRIGGER_DIST 时打印 [TRIGGER!]
//
// Step 4: 调整 ADAPTIVE_THRESH（可选）
//   若光照较暗识别失败，适当调大 ADAPTIVE_THRESH（默认 10，可尝试 15~20）
// ─────────────────────────────────────────────────────────────────────────────
#include <unitree/robot/channel/channel_factory.hpp>
#include <unitree/robot/go2/video/video_client.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/aruco.hpp>
#include <csignal>
#include <atomic>
#include <unistd.h>
#include "../src/perception/ArucoDetector.hpp"
#include "../src/perception/ArucoDetector.cpp"

// ── 调参区 ────────────────────────────────────────────────────────────────────
static const double TAG_SIZE         = 0.0714; // Tag 黑色区域物理边长(m)，10cm含白边实测约7.14cm
static const double FOCAL_LENGTH     = 600.0;  // 焦距(px)，标定前用估算值
static const double TRIGGER_DIST     = 1.5;    // 触发爬台阶的距离阈值(m)
static const int    TARGET_TAG_ID    = -1;      // 目标 Tag ID，-1 = 任意
static const double ADAPTIVE_THRESH  = 10.0;   // ArUco 内部二值化参数，光照差时调大
// ─────────────────────────────────────────────────────────────────────────────

std::atomic<bool> g_running{true};
void on_sigint(int) { g_running = false; }

int main(int argc, char** argv) {
    if (argc < 2) { fprintf(stderr, "Usage: sudo %s <iface>\n", argv[0]); return 1; }
    signal(SIGINT, on_sigint);

    unitree::robot::ChannelFactory::Instance()->Init(0, argv[1]);
    unitree::robot::go2::VideoClient vc;
    vc.SetTimeout(3.f); vc.Init();
    sleep(1);

    ArucoDetector detector(TAG_SIZE, FOCAL_LENGTH);

    std::cout << "ArUco detection started.\n"
              << "  TAG_SIZE=" << TAG_SIZE << "m  FOCAL=" << FOCAL_LENGTH
              << "  TRIGGER=" << TRIGGER_DIST << "m\n"
              << "  Ctrl+C to stop, Space to pause\n";

    std::vector<uint8_t> buf;
    while (g_running) {
        if (vc.GetImageSample(buf) != 0) { usleep(20000); continue; }
        cv::Mat frame = cv::imdecode(cv::Mat(buf), cv::IMREAD_COLOR);
        if (frame.empty()) { usleep(20000); continue; }

        cv::Mat vis = frame.clone();
        auto results = detector.detect(frame, &vis);

        if (results.empty()) {
            cv::putText(vis, "No Tag", {50,50}, cv::FONT_HERSHEY_SIMPLEX, 1, {0,0,255}, 2);
        }

        for (auto& r : results) {
            // 标定辅助：打印 pixelLen（由 TAG_SIZE * FOCAL / dist 反推）
            double px_est = (r.distance > 0) ? TAG_SIZE * FOCAL_LENGTH / r.distance : 0;
            printf("ID=%-3d  dist=%.3fm  pixelLen=%.1fpx  %s\n",
                   r.id, r.distance, px_est,
                   (TARGET_TAG_ID < 0 || r.id == TARGET_TAG_ID) ?
                   (r.distance < TRIGGER_DIST ? "[TRIGGER!]" : "[OK]") : "");
        }

        cv::imshow("ArUco", vis);
        char key = (char)cv::waitKey(1);
        if (key == 27) break;
        if (key == 32) cv::waitKey(0);
        usleep(20000);
    }
    return 0;
}
