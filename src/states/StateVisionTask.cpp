#include "StateVisionTask.hpp"
#include "core/RobotContext.hpp"
#include "core/StateIDs.hpp"
#include <iostream>

void StateVisionTask::enter(RobotContext& ctx) {
    std::cout << "[VisionTask] enter\n";
    ctx.sport->stop();
    confirm_count_ = 0;

    if (!model_loaded_) {
        std::string model = ctx.cfg["tasks"]["vision"]["model_path"].as<std::string>("models/yolo.onnx");
        float conf  = ctx.cfg["tasks"]["vision"]["conf_thresh"].as<float>(0.5f);
        int   size  = ctx.cfg["tasks"]["vision"]["input_size"].as<int>(640);
        try {
            detector_.load(model, conf, size);
            model_loaded_ = true;
            std::cout << "[VisionTask] model loaded: " << model << "\n";
        } catch (const std::exception& e) {
            std::cerr << "[VisionTask] model load failed: " << e.what() << "\n";
        }
    }
}

int StateVisionTask::update(RobotContext& ctx) {
    cv::Mat frame;
    if (!ctx.camera->grab(frame)) return STATE_STAY;

    if (!model_loaded_) return STATE_ARM_CONTROL;

    Detections dets = detector_.detect(frame);
    for (auto& d : dets)
        std::cout << "[VisionTask] " << d.label << " conf=" << d.confidence << "\n";

    if (!dets.empty()) {
        if (++confirm_count_ >= 3) return STATE_ARM_CONTROL;
    } else {
        confirm_count_ = 0;
    }
    return STATE_STAY;
}
