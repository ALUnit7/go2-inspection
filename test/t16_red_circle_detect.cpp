// t16_red_circle_detect: 地面红色圆形检测（检测平台触发）
// 无需连机器人，用海康相机调参
// 空格暂停，ESC退出，检测到红色圆时高亮显示
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <csignal>
#include <atomic>
#include <unistd.h>
#include "../src/perception/HikCamera.hpp"
#include "../src/perception/HikCamera.cpp"

std::atomic<bool> g_running{true};
void on_sigint(int) { g_running = false; }

int main() {
    signal(SIGINT, on_sigint);
    HikCamera cam;
    if (!cam.init()) return 1;

    // 滑动条
    int h1_lo=0,  h1_hi=15;
    int h2_lo=163,h2_hi=179;
    int s_lo=100, s_hi=255;
    int v_lo=80,  v_hi=255;
    int min_area = 3000;  // 最小圆面积（像素）
    int circularity_x100 = 70; // 圆形度阈值×100（0.7=较圆）

    const char* W = "Red Circle Detect";
    cv::namedWindow(W);
    cv::createTrackbar("H1_lo",  W, &h1_lo,  179);
    cv::createTrackbar("H1_hi",  W, &h1_hi,  179);
    cv::createTrackbar("H2_lo",  W, &h2_lo,  179);
    cv::createTrackbar("H2_hi",  W, &h2_hi,  179);
    cv::createTrackbar("S_lo",   W, &s_lo,   255);
    cv::createTrackbar("S_hi",   W, &s_hi,   255);
    cv::createTrackbar("V_lo",   W, &v_lo,   255);
    cv::createTrackbar("MinArea",W, &min_area,20000);
    cv::createTrackbar("Circle%",W, &circularity_x100, 100);

    while (g_running) {
        cv::Mat frame;
        if (!cam.grab(frame)) { usleep(20000); continue; }

        cv::Mat hsv, mask1, mask2, mask;
        cv::cvtColor(frame, hsv, cv::COLOR_BGR2HSV);
        cv::inRange(hsv, cv::Scalar(h1_lo,s_lo,v_lo), cv::Scalar(h1_hi,s_hi,v_hi), mask1);
        cv::inRange(hsv, cv::Scalar(h2_lo,s_lo,v_lo), cv::Scalar(h2_hi,s_hi,v_hi), mask2);
        mask = mask1 | mask2;

        // 形态学去噪
        cv::Mat k = cv::getStructuringElement(cv::MORPH_ELLIPSE, {9,9});
        cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, k);
        cv::morphologyEx(mask, mask, cv::MORPH_OPEN,  k);

        // 找轮廓，筛选圆形
        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        cv::Mat vis = frame.clone();
        bool detected = false;
        for (auto& c : contours) {
            double area = cv::contourArea(c);
            if (area < min_area) continue;
            double perim = cv::arcLength(c, true);
            double circ  = 4 * CV_PI * area / (perim * perim);
            if (circ < circularity_x100 / 100.0) continue;

            cv::Point2f center; float radius;
            cv::minEnclosingCircle(c, center, radius);
            cv::circle(vis, center, (int)radius, {0,0,255}, 3);
            cv::circle(vis, center, 5, {0,255,0}, -1);
            char buf[64];
            snprintf(buf, sizeof(buf), "circ=%.2f area=%.0f", circ, area);
            cv::putText(vis, buf, {(int)center.x-60,(int)center.y-int(radius)-10},
                        cv::FONT_HERSHEY_SIMPLEX, 0.7, {0,255,255}, 2);
            detected = true;
            printf("RED CIRCLE: center=(%.0f,%.0f) r=%.0f circ=%.2f\n",
                   center.x, center.y, radius, circ);
        }

        cv::putText(vis, detected ? "DETECTED!" : "no circle",
                    {30,50}, cv::FONT_HERSHEY_SIMPLEX, 1.5,
                    detected ? cv::Scalar{0,0,255} : cv::Scalar{0,255,0}, 3);

        // 拼接掩码
        cv::Mat mask_bgr;
        cv::cvtColor(mask, mask_bgr, cv::COLOR_GRAY2BGR);
        cv::Mat r0, r1, grid;
        cv::resize(vis,      r0, {vis.cols/2, vis.rows/2});
        cv::resize(mask_bgr, r1, {vis.cols/2, vis.rows/2});
        cv::hconcat(r0, r1, grid);
        cv::imshow(W, grid);

        char key = (char)cv::waitKey(1);
        if (key == 27) break;
        if (key == 32) {
            printf("HSV: H1(%d,%d) H2(%d,%d) S(%d,%d) V(%d,%d) area=%d circ=%.2f\n",
                   h1_lo,h1_hi, h2_lo,h2_hi, s_lo,s_hi, v_lo,v_hi,
                   min_area, circularity_x100/100.0);
            cv::waitKey(0);
        }
        usleep(20000);
    }
    return 0;
}
