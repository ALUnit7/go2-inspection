// t13_yolo_sign: YOLO11 警示标志识别（GO2 前置摄像头）
// 用法: sudo ./t13_yolo_sign <网卡名> [model_path]
//
// ── 类别 → 比赛动作映射 ────────────────────────────────────────────────────────
// 0: 当心辐射     → VuiClient 灯光闪烁三次
// 1: 当心触电     → Stretch()
// 2: 当心强氧化物 → Hello()
//
// ── Jetson NX GPU 加速（部署时）─────────────────────────────────────────────
// 需替换为 ONNX Runtime ARM64 + CUDA 版本：
// OrtCUDAProviderOptions cuda_opts{};
// session_opts.AppendExecutionProvider_CUDA(cuda_opts);
// ─────────────────────────────────────────────────────────────────────────────
#include <unitree/robot/channel/channel_factory.hpp>
#include <unitree/robot/go2/video/video_client.hpp>
#include <opencv2/highgui.hpp>
#include <csignal>
#include <atomic>
#include <unistd.h>
#include "../src/perception/YoloDetector.hpp"
#include "../src/perception/YoloDetector.cpp"

// ── 调参区 ────────────────────────────────────────────────────────────────────
static const std::string MODEL_PATH  = "/home/wzl/GO2_RCOM/models/warning_sign.onnx";
static const float CONF_THRESH       = 0.25f;
static const float NMS_THRESH        = 0.45f;
static const int   INPUT_SIZE        = 512;
// 类别顺序必须与训练时一致
static const std::vector<std::string> CLASSES = {"radiation", "electric", "oxidizer"};
// ─────────────────────────────────────────────────────────────────────────────

std::atomic<bool> g_running{true};
void on_sigint(int) { g_running = false; }

int main(int argc, char** argv) {
    if (argc < 2) { fprintf(stderr, "Usage: sudo %s <iface> [model]\n", argv[0]); return 1; }
    signal(SIGINT, on_sigint);

    std::string model = (argc > 2) ? argv[2] : MODEL_PATH;

    unitree::robot::ChannelFactory::Instance()->Init(0, argv[1]);
    unitree::robot::go2::VideoClient vc;
    vc.SetTimeout(3.f); vc.Init();
    sleep(1);

    YoloDetector detector(model, CLASSES, CONF_THRESH, NMS_THRESH, INPUT_SIZE);
    std::cout << "YOLO ready. model=" << model << "\n  Ctrl+C stop, Space pause\n";

    std::vector<uint8_t> buf;
    while (g_running) {
        if (vc.GetImageSample(buf) != 0) { usleep(20000); continue; }
        cv::Mat frame = cv::imdecode(cv::Mat(buf), cv::IMREAD_COLOR);
        if (frame.empty()) { usleep(20000); continue; }

        cv::Mat vis = frame.clone();
        auto dets = detector.detect(frame, &vis);

        for (auto& d : dets)
            printf("class=%d(%s) conf=%.2f box=[%d,%d,%d,%d]\n",
                   d.class_id, d.label.c_str(), d.conf,
                   d.box.x, d.box.y, d.box.width, d.box.height);

        cv::imshow("YOLO Sign", vis);
        char key = (char)cv::waitKey(1);
        if (key == 27) break;
        if (key == 32) cv::waitKey(0);
        usleep(20000);
    }
    return 0;
}
