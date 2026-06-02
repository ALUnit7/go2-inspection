#include "YoloDetector.hpp"
#include <onnxruntime_cxx_api.h>
#include <opencv2/imgproc.hpp>
#include <opencv2/dnn.hpp>
#include <cstring>

struct YoloDetector::Impl {
    Ort::Env            env;
    Ort::Session        session;
    std::string         in_name, out_name;
    std::vector<std::string> classes;
    float conf_thresh, nms_thresh;
    int   input_size;

    Impl(const std::string& model_path,
         const std::vector<std::string>& class_names,
         float conf, float nms, int sz)
        : env(ORT_LOGGING_LEVEL_WARNING, "yolo"),
          session(env, model_path.c_str(), Ort::SessionOptions{}),
          classes(class_names), conf_thresh(conf), nms_thresh(nms), input_size(sz)
    {
        Ort::AllocatorWithDefaultOptions alloc;
        in_name  = session.GetInputNameAllocated(0, alloc).get();
        out_name = session.GetOutputNameAllocated(0, alloc).get();
    }
};

YoloDetector::YoloDetector(const std::string& model_path,
                             const std::vector<std::string>& class_names,
                             float conf_thresh, float nms_thresh, int input_size)
    : impl_(new Impl(model_path, class_names, conf_thresh, nms_thresh, input_size)) {}

YoloDetector::~YoloDetector() { delete impl_; }

std::vector<YoloDet> YoloDetector::detect(const cv::Mat& bgr, cv::Mat* vis) {
    auto& I = *impl_;
    int sz = I.input_size;

    // 预处理：resize → RGB → float → CHW
    cv::Mat resized, f;
    cv::resize(bgr, resized, {sz, sz});
    cv::cvtColor(resized, resized, cv::COLOR_BGR2RGB);
    resized.convertTo(f, CV_32F, 1.0 / 255.0);

    std::vector<cv::Mat> chw(3);
    cv::split(f, chw);
    std::vector<float> input(3 * sz * sz);
    for (int c = 0; c < 3; c++)
        memcpy(input.data() + c * sz * sz, chw[c].ptr<float>(), sz * sz * sizeof(float));

    int64_t shape[] = {1, 3, sz, sz};
    auto mem = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    auto tensor = Ort::Value::CreateTensor<float>(mem, input.data(), input.size(), shape, 4);

    const char* in_names[]  = { I.in_name.c_str() };
    const char* out_names[] = { I.out_name.c_str() };
    auto results = I.session.Run(Ort::RunOptions{}, in_names, &tensor, 1, out_names, 1);

    // 解析输出 (1, features, anchors)
    auto dims    = results[0].GetTensorTypeAndShapeInfo().GetShape();
    int features = (int)dims[1], anchors = (int)dims[2];
    int num_cls  = features - 4;
    float* data  = results[0].GetTensorMutableData<float>();

    std::vector<cv::Rect> boxes; std::vector<float> confs; std::vector<int> ids;
    float sx = (float)bgr.cols / sz, sy = (float)bgr.rows / sz;

    for (int i = 0; i < anchors; i++) {
        int id = 0; float best = 0;
        for (int j = 0; j < num_cls; j++) {
            float s = data[(4 + j) * anchors + i];
            if (s > best) { best = s; id = j; }
        }
        if (best < I.conf_thresh) continue;
        float x = data[0*anchors+i], y = data[1*anchors+i];
        float w = data[2*anchors+i], h = data[3*anchors+i];
        boxes.push_back({(int)((x-w/2)*sx), (int)((y-h/2)*sy),
                         (int)(w*sx), (int)(h*sy)});
        confs.push_back(best); ids.push_back(id);
    }

    std::vector<int> keep;
    cv::dnn::NMSBoxes(boxes, confs, I.conf_thresh, I.nms_thresh, keep);

    std::vector<YoloDet> out;
    for (int k : keep) {
        std::string lbl = (ids[k] < (int)I.classes.size()) ? I.classes[ids[k]] : std::to_string(ids[k]);
        out.push_back({ids[k], lbl, confs[k], boxes[k]});
        if (vis) {
            cv::rectangle(*vis, boxes[k], {0,255,0}, 2);
            char buf[64]; snprintf(buf, sizeof(buf), "%s %.2f", lbl.c_str(), confs[k]);
            cv::putText(*vis, buf, {boxes[k].x, std::max(boxes[k].y-5,0)},
                        cv::FONT_HERSHEY_SIMPLEX, 0.7, {0,255,0}, 2);
        }
    }
    return out;
}
