#pragma once
#include <opencv2/core.hpp>
#include <unitree/robot/go2/video/video_client.hpp>

// VideoClient 封装：GetImageSample -> cv::Mat (BGR)
class CameraClient {
public:
    void init(float timeout = 3.f);
    // 返回 true 并填充 frame；失败返回 false
    bool grab(cv::Mat& frame);

private:
    unitree::robot::go2::VideoClient client_;
    std::vector<uint8_t> buf_;
};
