#include "ArucoDetector.hpp"
#include <opencv2/aruco.hpp>
#include <opencv2/imgproc.hpp>
#include <cmath>

ArucoDetector::ArucoDetector(double tag_size, double focal_length)
    : tag_size_(tag_size), focal_length_(focal_length)
{
    dict_   = cv::makePtr<cv::aruco::Dictionary>(
                  cv::aruco::getPredefinedDictionary(cv::aruco::DICT_6X6_250));
    params_ = cv::makePtr<cv::aruco::DetectorParameters>();
    params_->adaptiveThreshConstant = 10.0;
}

std::vector<TagResult> ArucoDetector::detect(const cv::Mat& frame, cv::Mat* vis) {
    cv::Mat gray;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

    std::vector<std::vector<cv::Point2f>> corners;
    std::vector<int> ids;
    cv::aruco::detectMarkers(gray, dict_, corners, ids, params_);

    std::vector<TagResult> results;
    for (size_t i = 0; i < ids.size(); i++) {
        const auto& c = corners[i];
        double px = (cv::norm(c[0]-c[1]) + cv::norm(c[1]-c[2]) +
                     cv::norm(c[2]-c[3]) + cv::norm(c[3]-c[0])) / 4.0;
        double dist = (px > 0) ? (tag_size_ * focal_length_) / px : -1;
        results.push_back({ids[i], dist});
    }

    if (vis && !ids.empty()) {
        cv::aruco::drawDetectedMarkers(*vis, corners, ids);
        for (size_t i = 0; i < results.size(); i++) {
            char buf[64];
            snprintf(buf, sizeof(buf), "ID:%d %.2fm", results[i].id, results[i].distance);
            cv::putText(*vis, buf, corners[i][0],
                        cv::FONT_HERSHEY_SIMPLEX, 0.8, {0,255,0}, 2);
        }
    }
    return results;
}
