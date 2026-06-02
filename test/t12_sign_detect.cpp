// t12_sign_detect: 警示标志识别测试（GO2 前置摄像头）
// 用法: sudo ./t12_sign_detect <网卡名>
//
// ── 使用步骤 ──────────────────────────────────────────────────────────────────
// Step 1: 调整 HSV 范围
//   运行程序，拖动 HSV调参 窗口的滑动条
//   目标：红色掩码图中标志区域为白色，背景为黑色
//   调好后将参数值记录到调参区
//
// Step 2: 确认类型映射
//   type=1（黑色像素多）和 type=2（白色像素多）各对应哪个比赛标志
//   对着每种标志各拍一次，观察 type 输出，记录对应关系
//
// Step 3: 调整 ratio_thresh（可选）
//   若 type 判断不稳定，调整 RATIO_THRESH 阈值
// ─────────────────────────────────────────────────────────────────────────────
#include <unitree/robot/channel/channel_factory.hpp>
#include <unitree/robot/go2/video/video_client.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <csignal>
#include <atomic>
#include <unistd.h>
#include "../src/perception/SignDetector.hpp"
#include "../src/perception/SignDetector.cpp"

std::atomic<bool> g_running{true};
void on_sigint(int) { g_running = false; }

int main(int argc, char** argv) {
    if (argc < 2) { fprintf(stderr, "Usage: sudo %s <iface>\n", argv[0]); return 1; }
    signal(SIGINT, on_sigint);

    unitree::robot::ChannelFactory::Instance()->Init(0, argv[1]);
    unitree::robot::go2::VideoClient vc;
    vc.SetTimeout(3.f); vc.Init();
    sleep(1);

    HsvRange cfg;  // 使用默认值，通过滑动条调整

    // 创建调参窗口
    const char* W_PARAM = "HSV调参";
    cv::namedWindow(W_PARAM);
    cv::createTrackbar("H1_lo", W_PARAM, &cfg.h1_lo, 179);
    cv::createTrackbar("H1_hi", W_PARAM, &cfg.h1_hi, 179);
    cv::createTrackbar("H2_lo", W_PARAM, &cfg.h2_lo, 179);
    cv::createTrackbar("H2_hi", W_PARAM, &cfg.h2_hi, 179);
    cv::createTrackbar("S_lo",  W_PARAM, &cfg.s_lo,  255);
    cv::createTrackbar("S_hi",  W_PARAM, &cfg.s_hi,  255);
    cv::createTrackbar("V_lo",  W_PARAM, &cfg.v_lo,  255);
    cv::createTrackbar("V_hi",  W_PARAM, &cfg.v_hi,  255);

    SignDetector detector(cfg);
    std::vector<uint8_t> buf;

    while (g_running) {
        if (vc.GetImageSample(buf) != 0) { usleep(20000); continue; }
        cv::Mat frame = cv::imdecode(cv::Mat(buf), cv::IMREAD_COLOR);
        if (frame.empty()) { usleep(20000); continue; }

        // 同步滑动条参数（detector 内部 cfg 是引用）
        detector.config() = cfg;

        // 生成掩码图用于显示
        cv::Mat hsv, mask1, mask2, mask;
        cv::cvtColor(frame, hsv, cv::COLOR_BGR2HSV);
        cv::inRange(hsv, cv::Scalar(cfg.h1_lo,cfg.s_lo,cfg.v_lo),
                         cv::Scalar(cfg.h1_hi,cfg.s_hi,cfg.v_hi), mask1);
        cv::inRange(hsv, cv::Scalar(cfg.h2_lo,cfg.s_lo,cfg.v_lo),
                         cv::Scalar(cfg.h2_hi,cfg.s_hi,cfg.v_hi), mask2);
        mask = mask1 | mask2;

        cv::Mat vis = frame.clone();
        SignResult res = detector.detect(frame, &vis);

        // 标注结果
        const char* labels[] = {"No sign", "Platform 1 (black>white)", "Platform 2 (white>black)"};
        cv::Scalar colors[]  = {{255,0,0}, {0,0,255}, {0,255,0}};
        int t = (res.valid && res.type >= 1 && res.type <= 2) ? res.type : 0;
        cv::putText(vis, labels[t], {30,50}, cv::FONT_HERSHEY_SIMPLEX, 1, colors[t], 2);
        if (res.valid)
            printf("type=%d  ratio=%.3f\n", res.type, res.ratio);

        // 拼接显示：左=原图+结果, 右=掩码
        cv::Mat mask_bgr;
        cv::cvtColor(mask, mask_bgr, cv::COLOR_GRAY2BGR);
        cv::Size sz(vis.cols/2, vis.rows/2);
        cv::Mat r0, r1, grid;
        cv::resize(vis,      r0, sz);
        cv::resize(mask_bgr, r1, sz);
        cv::hconcat(r0, r1, grid);
        cv::putText(grid, "Result", {10,30}, cv::FONT_HERSHEY_SIMPLEX, 0.8, {255,255,0}, 2);
        cv::putText(grid, "Mask",   {sz.width+10,30}, cv::FONT_HERSHEY_SIMPLEX, 0.8, {255,255,0}, 2);

        cv::imshow("Sign Detection", grid);
        char key = (char)cv::waitKey(1);
        if (key == 27) break;
        if (key == 32) {
            // 空格：打印当前 HSV 参数
            printf("HSV: H1(%d,%d) H2(%d,%d) S(%d,%d) V(%d,%d)\n",
                   cfg.h1_lo,cfg.h1_hi, cfg.h2_lo,cfg.h2_hi,
                   cfg.s_lo,cfg.s_hi, cfg.v_lo,cfg.v_hi);
            cv::waitKey(0);
        }
        usleep(20000);
    }
    return 0;
}
