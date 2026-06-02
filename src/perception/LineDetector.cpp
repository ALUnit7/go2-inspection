#include "LineDetector.hpp"
#include <opencv2/imgproc.hpp>
#include <cmath>

bool LineDetector::getLineEdge(const cv::Mat& bin, int row, int startX,
                                int& outLeft, int& outRight) const {
    int cols = bin.cols;
    if (startX < 0 || startX >= cols) return false;
    outLeft = outRight = -1;
    uchar center = bin.at<uchar>(row, startX);

    if (center == 255) {
        for (int x = startX; x >= 0; x--)
            if (bin.at<uchar>(row, x) == 0) { outLeft = x + 1; break; }
        for (int x = startX; x < cols; x++)
            if (bin.at<uchar>(row, x) == 0) { outRight = x - 1; break; }
    } else {
        int nearL = -1, nearR = -1;
        for (int x = startX - 1; x >= 0; x--)
            if (bin.at<uchar>(row, x) == 255) { nearL = x; break; }
        for (int x = startX + 1; x < cols; x++)
            if (bin.at<uchar>(row, x) == 255) { nearR = x; break; }
        if (nearL == -1 && nearR == -1) return false;
        if (nearL != -1 && nearR == -1) {
            for (int x = nearL; x >= 0; x--)
                if (bin.at<uchar>(row, x) == 0) { outLeft = x + 1; break; }
            outRight = nearL;
        } else if (nearR != -1 && nearL == -1) {
            for (int x = nearR; x < cols; x++)
                if (bin.at<uchar>(row, x) == 0) { outRight = x - 1; break; }
            outLeft = nearR;
        } else {
            if (startX - nearL <= nearR - startX) {
                for (int x = nearL; x >= 0; x--)
                    if (bin.at<uchar>(row, x) == 0) { outLeft = x + 1; break; }
                outRight = nearL;
            } else {
                for (int x = nearR; x < cols; x++)
                    if (bin.at<uchar>(row, x) == 0) { outRight = x - 1; break; }
                outLeft = nearR;
            }
        }
    }

    // P1修复：线延伸到图像边缘时用边缘代替，避免丢帧
    if (outLeft  == -1) outLeft  = 0;
    if (outRight == -1) outRight = cols - 1;
    return true;
}

LineResult LineDetector::detectImpl(const cv::Mat& bgr, cv::Mat* vis) {
    cv::Mat gray, binary;
    cv::cvtColor(bgr, gray, cv::COLOR_BGR2GRAY);
    cv::threshold(gray, binary, thresh_, 255, cv::THRESH_BINARY_INV);

    // 先闭运算（填充白色内部黑色空洞，抑制反光噪点）
    cv::Mat k_close = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(close_k_, close_k_));
    cv::dilate(binary, binary, k_close);
    cv::erode(binary, binary, k_close);
    // 再开运算（去除白色外部小噪点）
    cv::Mat k_open = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(open_k_, open_k_));
    cv::erode(binary, binary, k_open);
    cv::dilate(binary, binary, k_open);

    int h = binary.rows, w = binary.cols;
    int bot_row = h - 1;
    int top_row = (top_row_ < h) ? top_row_ : 0;

    // P3修复：底部行用上一帧中点作为起始点
    int start_cx = (last_cx_ > 0) ? last_cx_ : w / 2;

    int botL, botR;
    if (!getLineEdge(binary, bot_row, start_cx, botL, botR)) {
        // 上一帧位置也失败，回退到图像中心重试
        if (!getLineEdge(binary, bot_row, w / 2, botL, botR))
            return {0, 0, 0, 0, false, false};
    }

    int line_width = botR - botL;
    cv::Point midBot((botL + botR) / 2, bot_row);
    last_cx_ = midBot.x;  // 更新历史位置

    int topL, topR;
    bool top_ok = getLineEdge(binary, top_row, midBot.x, topL, topR);

    double angle = 0, length = 0;
    cv::Point midTop(midBot.x, top_row);  // 默认：顶部中点与底部对齐

    if (top_ok) {
        midTop = cv::Point((topL + topR) / 2, top_row);
        double dx = midBot.x - midTop.x;
        double dy = midBot.y - midTop.y;
        angle  = -std::atan2(dx, dy) * 180.0 / CV_PI;
        length = std::sqrt(dx * dx + dy * dy);
    }
    // P2修复：顶部行失败时 angle=0，只用 offset 控制，不丢帧

    // 横向偏移：线在右侧为负（需右转），左侧为正（需左转）
    double offset = -(midBot.x - w / 2.0);

    if (vis) {
        if (top_ok) {
            cv::circle(*vis, {topL, top_row}, 5, {0,0,255}, -1);
            cv::circle(*vis, {topR, top_row}, 5, {0,0,255}, -1);
            cv::line(*vis, {topL,top_row}, {botL,bot_row}, {0,0,255}, 2);
            cv::line(*vis, {topR,top_row}, {botR,bot_row}, {0,0,255}, 2);
            cv::line(*vis, midTop, midBot, {255,0,0}, 2);
        }
        cv::circle(*vis, {botL, bot_row}, 5, {0,255,0}, -1);
        cv::circle(*vis, {botR, bot_row}, 5, {0,255,0}, -1);
        // 图像中心线
        cv::line(*vis, {w/2, 0}, {w/2, h}, {0,200,0}, 1);
        cv::putText(*vis, "Angle:" + std::to_string(angle).substr(0,6),
                    {20,40}, cv::FONT_HERSHEY_SIMPLEX, 1, {255,255,0}, 2);
        cv::putText(*vis, "Offset:" + std::to_string((int)offset),
                    {20,80}, cv::FONT_HERSHEY_SIMPLEX, 1, {0,255,255}, 2);
        cv::putText(*vis, top_ok ? "TOP:OK" : "TOP:LOST",
                    {20,120}, cv::FONT_HERSHEY_SIMPLEX, 0.8,
                    top_ok ? cv::Scalar{0,255,0} : cv::Scalar{0,0,255}, 2);
    }
    return {angle, offset, length, line_width, true, top_ok};
}

LineResult LineDetector::detect(const cv::Mat& bgr) {
    return detectImpl(bgr, nullptr);
}

LineResult LineDetector::detectAndDraw(cv::Mat& vis) {
    return detectImpl(vis, &vis);
}
