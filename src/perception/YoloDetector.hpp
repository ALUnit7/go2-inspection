#pragma once
#include <opencv2/core.hpp>
#include <string>
#include <vector>

struct YoloDet {
    int         class_id;
    std::string label;
    float       conf;
    cv::Rect    box;
};

class YoloDetector {
public:
    // model_path:   .onnx 文件路径
    // class_names:  类别名列表，顺序与训练时一致
    // conf_thresh:  置信度阈值
    // nms_thresh:   NMS 阈值
    // input_size:   模型输入边长（正方形）
    YoloDetector(const std::string& model_path,
                 const std::vector<std::string>& class_names,
                 float conf_thresh = 0.25f,
                 float nms_thresh  = 0.45f,
                 int   input_size  = 512);
    ~YoloDetector();

    // 推理，返回所有检测结果，在 vis 上画框（vis=nullptr 不画）
    std::vector<YoloDet> detect(const cv::Mat& bgr, cv::Mat* vis = nullptr);

private:
    struct Impl;
    Impl* impl_;
};
