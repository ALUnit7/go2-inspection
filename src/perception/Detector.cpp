#include "Detector.hpp"
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <fstream>

void Detector::load(const std::string& model_path, float conf_thresh, int input_size) {
    conf_thresh_ = conf_thresh;
    input_size_  = input_size;
    net_ = cv::dnn::readNetFromONNX(model_path);
    net_.setPreferableBackend(cv::dnn::DNN_BACKEND_DEFAULT);
    net_.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);

    // 尝试加载同目录下的 labels.txt
    std::string label_path = model_path.substr(0, model_path.rfind('.')) + ".txt";
    std::ifstream f(label_path);
    std::string line;
    while (std::getline(f, line))
        if (!line.empty()) class_names_.push_back(line);
}

Detections Detector::detect(const cv::Mat& frame) {
    cv::Mat blob;
    cv::dnn::blobFromImage(frame, blob, 1.0/255.0,
        cv::Size(input_size_, input_size_), cv::Scalar(), true, false);
    net_.setInput(blob);

    std::vector<cv::Mat> outs;
    net_.forward(outs, net_.getUnconnectedOutLayersNames());

    Detections result;
    parse_outputs(outs, frame.size(), result);
    return result;
}

void Detector::parse_outputs(const std::vector<cv::Mat>& outs,
                              const cv::Size& orig, Detections& result) {
    // YOLOv8 输出格式: [1, 84, 8400]
    if (outs.empty()) return;
    cv::Mat data = outs[0]; // shape [1, 84, N] or [84, N]
    if (data.dims == 3) data = data.reshape(1, data.size[1]);

    float sx = (float)orig.width  / input_size_;
    float sy = (float)orig.height / input_size_;

    for (int i = 0; i < data.cols; ++i) {
        float* col = data.ptr<float>(0) + i;
        // cx,cy,w,h at rows 0-3; class scores at rows 4+
        int stride = data.rows > 1 ? data.step1() : 1;
        // 简化：直接按列访问
        cv::Mat col_mat = data.col(i);
        float cx = col_mat.at<float>(0) * sx;
        float cy = col_mat.at<float>(1) * sy;
        float bw = col_mat.at<float>(2) * sx;
        float bh = col_mat.at<float>(3) * sy;

        cv::Mat scores = col_mat.rowRange(4, col_mat.rows);
        double maxVal; int maxIdx[2];
        cv::minMaxIdx(scores, nullptr, &maxVal, nullptr, maxIdx);
        if ((float)maxVal < conf_thresh_) continue;

        Detection d;
        d.confidence = (float)maxVal;
        d.class_id   = maxIdx[0];
        d.label      = (d.class_id < (int)class_names_.size())
                       ? class_names_[d.class_id] : std::to_string(d.class_id);
        d.bbox = cv::Rect((int)(cx - bw/2), (int)(cy - bh/2), (int)bw, (int)bh);
        result.push_back(d);
    }
}
