# GO2 多媒体服务接口

> 更新时间：2025-11-21

---

## 接口概览

| 接口 | 方式 | 用途 |
|------|------|------|
| JPEG 拍照 | DDS / VideoClient RPC | 获取单张 720P JPEG 图像 |
| H264 图传 | 定向 UDP（GStreamer） | 实时视频流 |

> 图传推荐使用 UDP 接口，不推荐 DDS（DDS 传输 H264 需自行解码）。

**视频规格：** 1280×720，15Hz，水平视角 100°，垂直视角 56°

---

## JPEG 拍照（VideoClient）

### 接口

```cpp
// 头文件
#include <unitree/robot/go2/video/video_client.hpp>

int32_t VideoClient::GetImageSample(std::vector<uint8_t>& image_sample);
// 返回 0 成功，否则返回错误码
```

### 示例：循环拍照并保存

```cpp
#include <unitree/robot/go2/video/video_client.hpp>
#include <iostream>
#include <fstream>
#include <ctime>

int main(int argc, char **argv)
{
  if (argc < 2) { std::cout << "Usage: " << argv[0] << " networkInterface\n"; exit(-1); }

  unitree::robot::ChannelFactory::Instance()->Init(0, argv[1]);
  unitree::robot::go2::VideoClient video_client;
  video_client.SetTimeout(1.0f);
  video_client.Init();

  std::vector<uint8_t> image_sample;
  while (true)
  {
    if (video_client.GetImageSample(image_sample) == 0)
    {
      char buffer[80];
      time_t t; time(&t);
      strftime(buffer, sizeof(buffer), "%Y%m%d%H%M%S.jpg", localtime(&t));
      std::ofstream f(buffer, std::ios::binary);
      f.write(reinterpret_cast<const char*>(image_sample.data()), image_sample.size());
      std::cout << "Saved: " << buffer << std::endl;
    }
    sleep(3);
  }
  return 0;
}
```

---

## H264 图传（GStreamer UDP）

### 命令行直接查看

```bash
gst-launch-1.0 udpsrc address=230.1.1.1 port=1720 multicast-iface=<interface_name> \
  ! queue ! application/x-rtp, media=video, encoding-name=H264 \
  ! rtph264depay ! h264parse ! avdec_h264 ! videoconvert ! autovideosink
```

> `<interface_name>` 替换为连接 Go2 的网卡名，如 `eth0`

### 安装 GStreamer

```bash
sudo apt-get install libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev \
  libgstreamer-plugins-bad1.0-dev gstreamer1.0-plugins-base gstreamer1.0-plugins-good \
  gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly gstreamer1.0-libav gstreamer1.0-tools \
  gstreamer1.0-x gstreamer1.0-alsa gstreamer1.0-gl gstreamer1.0-gtk3 gstreamer1.0-pulseaudio
```

### 编译支持 GStreamer 的 OpenCV

```bash
# 下载 OpenCV 4.1.1 源码后：
mkdir build && cd build
cmake-gui ..
# 勾选 WITH_GSTREAMER（和 WITH_CUDA 可选）
# Configure → Generate
make && sudo make install
```

验证 GStreamer 是否启用：

```cpp
#include <opencv2/opencv.hpp>
int main() { std::cout << cv::getBuildInformation(); }
// 查看输出中 Video I/O: GStreamer 是否为 YES
```

### OpenCV 拉流

**C++：**

```cpp
#include <opencv2/opencv.hpp>
using namespace cv;

int main()
{
  VideoCapture cap(
    "udpsrc address=230.1.1.1 port=1720 multicast-iface=<interface_name> "
    "! application/x-rtp, media=video, encoding-name=H264 "
    "! rtph264depay ! h264parse ! avdec_h264 ! videoconvert "
    "! video/x-raw,width=1280,height=720,format=BGR ! appsink drop=1",
    CAP_GSTREAMER);

  if (!cap.isOpened()) { std::cerr << "VideoCapture not opened\n"; exit(-1); }

  Mat frame;
  while (true) {
    cap.read(frame);
    imshow("receiver", frame);
    if (waitKey(1) == 27) break;
  }
  return 0;
}
```

**Python：**

```python
import cv2

pipeline = (
    "udpsrc address=230.1.1.1 port=1720 multicast-iface=<interface_name> "
    "! application/x-rtp, media=video, encoding-name=H264 "
    "! rtph264depay ! h264parse ! avdec_h264 ! videoconvert "
    "! video/x-raw,width=1280,height=720,format=BGR ! appsink drop=1"
)
cap = cv2.VideoCapture(pipeline, cv2.CAP_GSTREAMER)

while cap.isOpened():
    ret, frame = cap.read()
    if ret:
        cv2.imshow("Go2 Camera", frame)
        if cv2.waitKey(25) & 0xFF == ord('q'):
            break

cap.release()
cv2.destroyAllWindows()
```
