#pragma once
#include <opencv2/core.hpp>
#include <vector>

struct LineResult {
    double angle;       // 偏角(度)：右倾为负，左倾为正
    double offset;      // 横向偏移(像素)：线在右侧为负，左侧为正
    double length;      // 向量长度（回归用，兼容旧接口）
    int    line_width;  // 底部行线宽
    bool   valid;
    bool   top_ok;      // 回归点数足够为true，否则false
};

class LineDetector {
public:
    explicit LineDetector(int threshold = 67, int top_row = 800,
                          int close_k = 12, int open_k = 5,
                          int n_rows = 12, double ransac_thresh = 30.0)
        : thresh_(threshold), top_row_(top_row),
          close_k_(close_k), open_k_(open_k),
          n_rows_(n_rows), ransac_thresh_(ransac_thresh),
          last_cx_(-1) {}

    LineResult detect(const cv::Mat& bgr);
    LineResult detectAndDraw(cv::Mat& vis);

private:
    int    thresh_, top_row_, close_k_, open_k_;
    int    n_rows_;           // 采样行数
    double ransac_thresh_;    // 离群点残差阈值（像素）
    int    last_cx_;

    bool getLineEdge(const cv::Mat& bin, int row, int startX,
                     int& outLeft, int& outRight) const;

    // 线性回归：返回 {slope, intercept}，valid_mask 标记内点
    bool linearRegress(const std::vector<cv::Point>& pts,
                       double& slope, double& intercept,
                       std::vector<bool>& valid_mask) const;

    LineResult detectImpl(const cv::Mat& bgr, cv::Mat* vis);
};
