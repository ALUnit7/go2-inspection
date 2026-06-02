#pragma once
#include "DetectionResult.hpp"
#include <string>
#include <opencv2/dnn.hpp>

// YOLO 推理封装（OpenCV DNN 后端，可替换为 ONNX Runtime / TensorRT）
class Detector {
public:
    void load(const std::string& model_path, float conf_thresh = 0.5f, int input_size = 640);
    Detections detect(const cv::Mat& frame);

private:
    cv::dnn::Net net_;
    float conf_thresh_{0.5f};
    int   input_size_{640};
    std::vector<std::string> class_names_;

    void parse_outputs(const std::vector<cv::Mat>& outs,
                       const cv::Size& orig, Detections& result);
};
