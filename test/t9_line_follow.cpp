// t9_line_follow: 寻迹视觉调试（滑动条调参 + 帧率）
// 空格暂停，ESC退出
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <csignal>
#include <atomic>
#include <chrono>
#include <unistd.h>
#include "../src/perception/LineDetector.hpp"
#include "../src/perception/LineDetector.cpp"
#include "../src/perception/HikCamera.hpp"
#include "../src/perception/HikCamera.cpp"

std::atomic<bool> g_running{true};
void on_sigint(int) { g_running = false; }

int main() {
    signal(SIGINT, on_sigint);

    HikCamera cam;
    if (!cam.init()) return 1;

    // 滑动条变量
    int thresh      = 67;
    int kp_x1000    = 8;
    int kp2_x10000  = 3;
    int top_row     = 800;
    int speed_x100  = 25;
    int exposure    = 0;
    int gain_x10    = 0;
    int close_k     = 7;   // 闭运算核（填充白色内黑色空洞）
    int open_k      = 5;   // 开运算核（去除外部噪点）

    const char* WIN = "1-Original+Result";
    cv::namedWindow(WIN);
    cv::createTrackbar("Thresh",       WIN, &thresh,      255);
    cv::createTrackbar("kp x1000",     WIN, &kp_x1000,    50);
    cv::createTrackbar("kp2 x10000",   WIN, &kp2_x10000,  50);
    cv::createTrackbar("TopRow",       WIN, &top_row,    1079);
    cv::createTrackbar("Speed x100",   WIN, &speed_x100,   80);
    cv::createTrackbar("Exp(x100us)",  WIN, &exposure,   1000);
    cv::createTrackbar("Gain x10",     WIN, &gain_x10,    200);
    cv::createTrackbar("CloseK",       WIN, &close_k,      30);
    cv::createTrackbar("OpenK",        WIN, &open_k,       30);

    int prev_exposure = -1, prev_gain = -1;
    auto t_last = std::chrono::steady_clock::now();
    int frame_count = 0;
    float fps = 0;

    while (g_running) {
        cv::Mat frame;
        if (!cam.grab(frame)) { usleep(20000); continue; }

        if (exposure != prev_exposure) {
            cam.setExposure(exposure > 0 ? exposure * 100.f : -1);
            prev_exposure = exposure;
        }
        if (gain_x10 != prev_gain) {
            cam.setGain(gain_x10 > 0 ? gain_x10 / 10.f : -1);
            prev_gain = gain_x10;
        }

        frame_count++;
        auto now = std::chrono::steady_clock::now();
        float elapsed = std::chrono::duration<float>(now - t_last).count();
        if (elapsed >= 1.0f) { fps = frame_count / elapsed; frame_count = 0; t_last = now; }

        float kp    = kp_x1000 / 1000.f;
        float kp2   = kp2_x10000 / 10000.f;
        float speed = speed_x100 / 100.f;

        LineDetector detector(thresh, top_row, close_k > 1 ? close_k : 1, open_k > 1 ? open_k : 1);
        cv::Mat vis = frame.clone();
        LineResult res = detector.detectAndDraw(vis);

        char fps_str[32];
        snprintf(fps_str, sizeof(fps_str), "FPS: %.1f", fps);
        cv::putText(vis, fps_str, {vis.cols-160, 40},
                    cv::FONT_HERSHEY_SIMPLEX, 1, {0,255,255}, 2);

        if (res.valid) {
            // vyaw = angle控制 + offset控制（两者叠加）
            float vyaw = (float)(res.angle * kp) + (float)(res.offset * kp2);
            printf("angle=%6.2f  offset=%6.1f  vyaw=%6.3f  fps=%.1f\n",
                   res.angle, res.offset, vyaw, fps);
        } else {
            printf("Line lost!  fps=%.1f\n", fps);
        }

        cv::Mat gray, binary, morphed;
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
        cv::threshold(gray, binary, thresh, 255, cv::THRESH_BINARY_INV);
        morphed = binary.clone();
        int ck = close_k > 1 ? close_k : 1;
        int ok = open_k  > 1 ? open_k  : 1;
        cv::Mat kc = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(ck,ck));
        cv::Mat ko = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(ok,ok));
        cv::dilate(morphed, morphed, kc); cv::erode(morphed, morphed, kc);  // 闭
        cv::erode(morphed, morphed, ko);  cv::dilate(morphed, morphed, ko); // 开

        // 井字格：2×2 拼接到一个窗口
        // gray 和 morphed 转为 BGR 才能和彩色图拼接
        cv::Mat gray_bgr, binary_bgr, morphed_bgr;
        cv::cvtColor(gray,    gray_bgr,    cv::COLOR_GRAY2BGR);
        cv::cvtColor(binary,  binary_bgr,  cv::COLOR_GRAY2BGR);
        cv::cvtColor(morphed, morphed_bgr, cv::COLOR_GRAY2BGR);

        // 缩放到统一尺寸（原图可能很大，缩小一半显示）
        cv::Size cell(vis.cols/2, vis.rows/2);
        cv::Mat r0, r1, r2, r3;
        cv::resize(vis,         r0, cell);
        cv::resize(gray_bgr,    r1, cell);
        cv::resize(binary_bgr,  r2, cell);
        cv::resize(morphed_bgr, r3, cell);

        // 标注各格标题
        cv::putText(r1, "Gray",    {10,40}, cv::FONT_HERSHEY_SIMPLEX, 1, {0,255,255}, 2);
        cv::putText(r2, "Binary",  {10,40}, cv::FONT_HERSHEY_SIMPLEX, 1, {0,255,255}, 2);
        cv::putText(r3, "Morphed", {10,40}, cv::FONT_HERSHEY_SIMPLEX, 1, {0,255,255}, 2);

        cv::Mat top, bot, grid;
        cv::hconcat(r0, r1, top);
        cv::hconcat(r2, r3, bot);
        cv::vconcat(top, bot, grid);

        cv::imshow(WIN, grid);
        char key = (char)cv::waitKey(1);
        if (key == 27) break;
        if (key == 32) cv::waitKey(0);
        usleep(20000);
    }
    return 0;
}
