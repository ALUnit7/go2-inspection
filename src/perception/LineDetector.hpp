#pragma once
#include <opencv2/core.hpp>

struct LineResult {
    double angle;
    double offset;
    double length;
    int    line_width;  // 底部行线宽（像素），十字路口时异常大
    bool   valid;
    bool   top_ok;
};

class LineDetector {
public:
    explicit LineDetector(int threshold = 67, int top_row = 800,
                          int close_k = 7, int open_k = 5)
        : thresh_(threshold), top_row_(top_row),
          close_k_(close_k), open_k_(open_k), last_cx_(-1) {}

    LineResult detect(const cv::Mat& bgr);
    LineResult detectAndDraw(cv::Mat& vis);

private:
    int thresh_;
    int top_row_;
    int close_k_;  // 闭运算核大小（填充白色内部黑色空洞）
    int open_k_;   // 开运算核大小（去除外部小噪点）
    int last_cx_;

    bool getLineEdge(const cv::Mat& bin, int row, int startX,
                     int& outLeft, int& outRight) const;
    LineResult detectImpl(const cv::Mat& bgr, cv::Mat* vis);
};
