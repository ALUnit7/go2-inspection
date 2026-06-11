// t14_obstacle_detect: 障碍物检测调试（海康工业相机）
// 无需连机器人，用于调整 near_row/far_row/white_thresh 参数
// 空格暂停，ESC退出，检测到障碍物时 ROI 变红
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <csignal>
#include <atomic>
#include <unistd.h>
#include "../src/perception/LineDetector.hpp"
#include "../src/perception/LineDetector.cpp"
#include "../src/perception/ObstacleDetector.hpp"
#include "../src/perception/ObstacleDetector.cpp"
#include "../src/perception/HikCamera.hpp"
#include "../src/perception/HikCamera.cpp"

std::atomic<bool> g_running{true};
void on_sigint(int) { g_running = false; }

int main() {
    signal(SIGINT, on_sigint);

    HikCamera cam;
    if (!cam.init()) return 1;

    // 滑动条变量
    int thresh    = 67;
    int close_k   = 12;
    int open_k    = 5;
    int near_row  = 1000;
    int far_row   = 740;
    int w_thresh  = 50;
    int confirm   = 3;
    int exposure  = 0;    // 0=自动，>0=固定值（单位：100us，如50=5ms）
    int gain_x10  = 0;    // 0=自动，>0=固定增益（单位：0.1，如20=2.0）

    const char* WIN = "Obstacle Detect";
    cv::namedWindow(WIN);
    cv::createTrackbar("Thresh",    WIN, &thresh,   255);
    cv::createTrackbar("CloseK",    WIN, &close_k,   30);
    cv::createTrackbar("OpenK",     WIN, &open_k,    30);
    cv::createTrackbar("NearRow",   WIN, &near_row, 1079);
    cv::createTrackbar("FarRow",    WIN, &far_row,  1079);
    cv::createTrackbar("WThresh%",  WIN, &w_thresh,  100);
    cv::createTrackbar("Confirm",   WIN, &confirm,    10);
    cv::createTrackbar("Exp(x100us)",WIN,&exposure, 1000); // 0=自动，100=10ms
    cv::createTrackbar("Gain x10",  WIN, &gain_x10,  200); // 0=自动，20=2.0

    int prev_exp=-1, prev_gain=-1;
    while (g_running) {
        cv::Mat frame;
        if (!cam.grab(frame)) { usleep(20000); continue; }

        if (exposure != prev_exp) {
            cam.setExposure(exposure > 0 ? exposure * 100.f : -1);
            prev_exp = exposure;
        }
        if (gain_x10 != prev_gain) {
            cam.setGain(gain_x10 > 0 ? gain_x10 / 10.f : -1);
            prev_gain = gain_x10;
        }

        // 二值化（与 LineDetector 一致）
        cv::Mat gray, binary;
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
        cv::threshold(gray, binary, thresh, 255, cv::THRESH_BINARY_INV);
        int ck = std::max(close_k, 1), ok = std::max(open_k, 1);
        cv::Mat kc = cv::getStructuringElement(cv::MORPH_RECT, {ck,ck});
        cv::Mat ko = cv::getStructuringElement(cv::MORPH_RECT, {ok,ok});
        cv::dilate(binary, binary, kc); cv::erode(binary, binary, kc);
        cv::erode(binary, binary, ko);  cv::dilate(binary, binary, ko);

        ObstacleConfig cfg;
        cfg.near_row       = std::min(near_row, binary.rows - 1);
        cfg.far_row        = std::min(far_row,  cfg.near_row - 1);
        cfg.white_thresh   = w_thresh / 100.f;
        cfg.confirm_frames = confirm;

        ObstacleDetector detector(cfg);
        cv::Mat vis = frame.clone();
        bool detected = detector.detect(binary, &vis);

        // 状态显示
        cv::putText(vis, detected ? "OBSTACLE!" : "clear",
                    {30, 50}, cv::FONT_HERSHEY_SIMPLEX, 1.5,
                    detected ? cv::Scalar{0,0,255} : cv::Scalar{0,255,0}, 3);

        // 拼接二值图
        cv::Mat bin_bgr;
        cv::cvtColor(binary, bin_bgr, cv::COLOR_GRAY2BGR);
        cv::Size sz(vis.cols/2, vis.rows/2);
        cv::Mat r0, r1, grid;
        cv::resize(vis,     r0, sz);
        cv::resize(bin_bgr, r1, sz);
        cv::hconcat(r0, r1, grid);
        cv::putText(grid, "Result",  {10,30}, cv::FONT_HERSHEY_SIMPLEX, 0.8, {255,255,0}, 2);
        cv::putText(grid, "Binary",  {sz.width+10,30}, cv::FONT_HERSHEY_SIMPLEX, 0.8, {255,255,0}, 2);

        cv::imshow(WIN, grid);
        char key = (char)cv::waitKey(1);
        if (key == 27) break;
        if (key == 32) cv::waitKey(0);
        usleep(20000);
    }
    return 0;
}
