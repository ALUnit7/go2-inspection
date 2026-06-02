#include "CameraClient.hpp"
#include <opencv2/imgcodecs.hpp>
#include <iostream>

void CameraClient::init(float timeout) {
    client_.SetTimeout(timeout);
    client_.Init();
}

bool CameraClient::grab(cv::Mat& frame) {
    int code = client_.GetImageSample(buf_);
    if (code != 0) {
        std::cerr << "[Camera] GetImageSample error=" << code << "\n";
        return false;
    }
    frame = cv::imdecode(cv::Mat(buf_), cv::IMREAD_COLOR);
    return !frame.empty();
}
