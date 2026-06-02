// t8_camera: VideoClient 读取单帧，保存为 /tmp/go2_frame.jpg
// 用法: sudo ./t8_camera <网卡名>
#include <unitree/robot/channel/channel_factory.hpp>
#include <unitree/robot/go2/video/video_client.hpp>
#include <opencv2/imgcodecs.hpp>
#include <iostream>
#include <unistd.h>

int main(int argc, char** argv) {
    if (argc < 2) { std::cerr << "Usage: " << argv[0] << " <iface>\n"; return 1; }

    unitree::robot::ChannelFactory::Instance()->Init(0, argv[1]);
    unitree::robot::go2::VideoClient vc;
    vc.SetTimeout(3.f); vc.Init();
    sleep(1);

    std::vector<uint8_t> buf;
    int code = vc.GetImageSample(buf);
    if (code != 0) { std::cerr << "GetImageSample error=" << code << "\n"; return 1; }

    cv::Mat img = cv::imdecode(cv::Mat(buf), cv::IMREAD_COLOR);
    if (img.empty()) { std::cerr << "imdecode failed\n"; return 1; }

    cv::imwrite("/tmp/go2_frame.jpg", img);
    std::cout << "Saved /tmp/go2_frame.jpg  size=" << img.cols << "x" << img.rows << "\n";
    return 0;
}
