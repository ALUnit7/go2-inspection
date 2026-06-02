#pragma once
#include <string>
#include <vector>
#include <opencv2/core.hpp>

struct Detection {
    int         class_id;
    std::string label;
    float       confidence;
    cv::Rect    bbox;
};

using Detections = std::vector<Detection>;
