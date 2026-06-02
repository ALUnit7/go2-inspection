# 海康工业相机 MVS SDK 调用教程（C++，零基础）

> SDK 版本：V4.8.0 | 系统：Ubuntu 22.04 | 语言：C++17

---

## 一、SDK 目录结构

```
/opt/MVS/
├── include/                    # 头文件
│   ├── MvCameraControl.h       # 主头文件，所有 API 都在这里
│   ├── CameraParams.h          # 枚举、结构体定义
│   ├── MvErrorDefine.h         # 错误码定义
│   └── PixelType.h             # 像素格式枚举
├── lib/
│   └── 64/
│       └── libMvCameraControl.so   # 动态库（64位）
├── bin/
│   └── MVS.sh                  # 图形化调试工具（强烈推荐先用它调参）
└── Samples/64/C++/             # 官方示例
    └── AreaScanCamera/
        ├── Trigger_Image/          # 触发采图
        ├── Trigger_ImageCallback/  # 回调采图
        └── SetParam/               # 参数设置
```

---

## 二、环境配置

### 注册动态库（只需执行一次）

```bash
echo "/opt/MVS/lib/64" | sudo tee /etc/ld.so.conf.d/mvs.conf
sudo ldconfig
```

### CMake 配置

```cmake
target_include_directories(your_target PRIVATE /opt/MVS/include)
target_link_libraries(your_target PRIVATE /opt/MVS/lib/64/libMvCameraControl.so)
```

### 头文件引入

```cpp
#include "MvCameraControl.h"   // 所有 API
#include "PixelType.h"         // 像素格式（PixelType_Gvsp_*）
```

---

## 三、核心概念

**句柄（handle）**：操作相机的"钥匙"，类型是 `void*`，所有 API 都需要传入它。

**返回值**：所有 API 返回 `int`，`MV_OK`（即0）表示成功，其他值是错误码（见 `/opt/MVS/include/MvErrorDefine.h`）。

**相机连接方式**：本项目使用 USB3 工业相机（也支持 GigE）。

---

## 四、完整调用流程

```
枚举设备 → 创建句柄 → 打开设备 → 设置参数 → 开始采图 → 取帧循环 → 停止采图 → 关闭销毁
```

### 4.1 枚举设备

```cpp
MV_CC_DEVICE_INFO_LIST devList{};
// MV_GIGE_DEVICE=GigE相机, MV_USB_DEVICE=USB3相机, 两者可用|组合
int ret = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &devList);
if (ret != MV_OK || devList.nDeviceNum == 0) {
    printf("未找到设备\n");
    return -1;
}
printf("找到 %d 台相机\n", devList.nDeviceNum);

// 打印每台相机信息
for (unsigned i = 0; i < devList.nDeviceNum; i++) {
    auto* info = devList.pDeviceInfo[i];
    if (info->nTLayerType == MV_USB_DEVICE)
        printf("[%d] USB: %s\n", i, info->SpecialInfo.stUsb3VInfo.chModelName);
    else if (info->nTLayerType == MV_GIGE_DEVICE)
        printf("[%d] GigE: %s\n", i, info->SpecialInfo.stGigEInfo.chModelName);
}
```

### 4.2 创建句柄 & 打开设备

```cpp
void* handle = nullptr;
// 使用第0台相机（索引从0开始）
MV_CC_CreateHandle(&handle, devList.pDeviceInfo[0]);
MV_CC_OpenDevice(handle);  // 默认独占模式
```

### 4.3 设置参数

所有参数通过字符串 key 设置，key 名称见下表。

#### 触发模式（连续采图 vs 触发采图）

```cpp
// 关闭触发，进入连续采图模式（最常用）
MV_CC_SetEnumValue(handle, "TriggerMode", 0);  // 0=off, 1=on

// 若需要软触发：
// MV_CC_SetEnumValue(handle, "TriggerMode", 1);
// MV_CC_SetEnumValue(handle, "TriggerSource", 7); // 7=软触发
// MV_CC_CommandExecute(handle, "TriggerSoftware");
```

#### 曝光时间

```cpp
// 关闭自动曝光，设置固定曝光时间
MV_CC_SetEnumValue(handle, "ExposureAuto", 0);       // 0=off
MV_CC_SetFloatValue(handle, "ExposureTime", 5000.0f); // 单位：微秒（5ms）

// 开启自动曝光：
// MV_CC_SetEnumValue(handle, "ExposureAuto", 2);     // 2=continuous
```

#### 增益

```cpp
// 关闭自动增益，设置固定增益
MV_CC_SetEnumValue(handle, "GainAuto", 0);       // 0=off
MV_CC_SetFloatValue(handle, "Gain", 1.0f);       // 范围因型号而异，通常 0~24 dB

// 开启自动增益：
// MV_CC_SetEnumValue(handle, "GainAuto", 2);
```

#### 白平衡（彩色相机）

```cpp
MV_CC_SetEnumValue(handle, "BalanceWhiteAuto", 2); // 2=连续自动, 0=关闭
```

#### 分辨率（只读，用于分配缓冲区）

```cpp
MVCC_INTVALUE val{};
MV_CC_GetIntValue(handle, "Width",  &val); int w = val.nCurValue;
MV_CC_GetIntValue(handle, "Height", &val); int h = val.nCurValue;
printf("分辨率: %d x %d\n", w, h);
```

#### GigE 专用：设置最优包大小

```cpp
// GigE 相机必须设置，否则帧率低
int nPacketSize = MV_CC_GetOptimalPacketSize(handle);
if (nPacketSize > 0)
    MV_CC_SetIntValue(handle, "GevSCPSPacketSize", nPacketSize);
```

### 4.4 开始采图

```cpp
MV_CC_StartGrabbing(handle);
```

### 4.5 取帧（轮询方式）

```cpp
std::vector<unsigned char> buf(w * h * 3);  // 预分配缓冲（RGB最大3字节/像素）
MV_FRAME_OUT_INFO_EX frameInfo{};

while (running) {
    int ret = MV_CC_GetOneFrameTimeout(
        handle,
        buf.data(),
        (unsigned int)buf.size(),
        &frameInfo,
        1000  // 超时毫秒
    );

    if (ret != MV_OK) {
        printf("取帧失败: 0x%X\n", ret);
        continue;
    }

    printf("帧号: %d, 宽: %d, 高: %d, 格式: 0x%X\n",
           frameInfo.nFrameNum, frameInfo.nWidth,
           frameInfo.nHeight, frameInfo.enPixelType);

    // 处理图像数据 buf.data()...
}
```

### 4.6 转为 OpenCV Mat

```cpp
#include <opencv2/imgproc.hpp>

cv::Mat toMat(unsigned char* data, const MV_FRAME_OUT_INFO_EX& info, void* handle) {
    cv::Mat result;
    if (info.enPixelType == PixelType_Gvsp_Mono8) {
        // 灰度图
        result = cv::Mat(info.nHeight, info.nWidth, CV_8UC1, data).clone();
    } else if (info.enPixelType == PixelType_Gvsp_RGB8_Packed) {
        // RGB → BGR
        cv::Mat rgb(info.nHeight, info.nWidth, CV_8UC3, data);
        cv::cvtColor(rgb, result, cv::COLOR_RGB2BGR);
    } else if (info.enPixelType == PixelType_Gvsp_BGR8_Packed) {
        result = cv::Mat(info.nHeight, info.nWidth, CV_8UC3, data).clone();
    } else {
        // Bayer 或其他格式，用 SDK 转换
        std::vector<unsigned char> dst(info.nWidth * info.nHeight * 3);
        MV_CC_PIXEL_CONVERT_PARAM_EX cvt;
        cvt.nWidth         = info.nWidth;
        cvt.nHeight        = info.nHeight;
        cvt.enSrcPixelType = info.enPixelType;
        cvt.pSrcData       = data;
        cvt.nSrcDataLen    = info.nFrameLen;
        cvt.enDstPixelType = PixelType_Gvsp_BGR8_Packed;
        cvt.pDstBuffer     = dst.data();
        cvt.nDstBufferSize = (unsigned int)dst.size();
        MV_CC_ConvertPixelTypeEx(handle, &cvt);
        result = cv::Mat(info.nHeight, info.nWidth, CV_8UC3, dst.data()).clone();
    }
    return result;
}
```

### 4.7 停止 & 释放

```cpp
MV_CC_StopGrabbing(handle);
MV_CC_CloseDevice(handle);
MV_CC_DestroyHandle(handle);
handle = nullptr;
```

---

## 五、完整最小示例

```cpp
#include <opencv2/highgui.hpp>
#include "MvCameraControl.h"
#include <vector>

int main() {
    // 1. 枚举
    MV_CC_DEVICE_INFO_LIST devList{};
    MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &devList);
    if (!devList.nDeviceNum) { printf("无设备\n"); return 1; }

    // 2. 打开
    void* h = nullptr;
    MV_CC_CreateHandle(&h, devList.pDeviceInfo[0]);
    MV_CC_OpenDevice(h);

    // 3. 参数
    MV_CC_SetEnumValue(h, "TriggerMode", 0);  // 连续采图
    MV_CC_SetEnumValue(h, "ExposureAuto", 2); // 自动曝光

    // 4. 读取分辨率
    MVCC_INTVALUE val{};
    MV_CC_GetIntValue(h, "Width",  &val); int w = val.nCurValue;
    MV_CC_GetIntValue(h, "Height", &val); int hh = val.nCurValue;
    std::vector<unsigned char> buf(w * hh * 3);

    // 5. 采图
    MV_CC_StartGrabbing(h);
    MV_FRAME_OUT_INFO_EX info{};

    while (true) {
        if (MV_CC_GetOneFrameTimeout(h, buf.data(), buf.size(), &info, 1000) == MV_OK) {
            // Mono8 示例
            cv::Mat frame(info.nHeight, info.nWidth, CV_8UC1, buf.data());
            cv::imshow("Camera", frame);
        }
        if (cv::waitKey(1) == 27) break;
    }

    // 6. 释放
    MV_CC_StopGrabbing(h);
    MV_CC_CloseDevice(h);
    MV_CC_DestroyHandle(h);
    return 0;
}
```

---

## 六、常用参数 Key 速查

| Key | 类型 | 说明 |
|-----|------|------|
| `TriggerMode` | Enum | 0=连续, 1=触发 |
| `TriggerSource` | Enum | 0=Line0, 7=软触发 |
| `ExposureAuto` | Enum | 0=关, 2=连续自动 |
| `ExposureTime` | Float | 曝光时间（微秒） |
| `GainAuto` | Enum | 0=关, 2=连续自动 |
| `Gain` | Float | 增益值（dB） |
| `BalanceWhiteAuto` | Enum | 0=关, 2=连续自动 |
| `Width` | Int（只读） | 图像宽度 |
| `Height` | Int（只读） | 图像高度 |
| `PixelFormat` | Enum | 像素格式 |
| `AcquisitionFrameRate` | Float | 帧率（需先开启帧率控制） |
| `GevSCPSPacketSize` | Int | GigE包大小 |

---

## 七、调试建议

1. **先用图形工具调好参数再写代码**

```bash
/opt/MVS/bin/MVS.sh
```

在 MVS 软件里实时预览，调整曝光、增益、白平衡，记录满意的参数值后写入代码。

2. **错误码查询**

```bash
grep "0x8001" /opt/MVS/include/MvErrorDefine.h
```

3. **本项目的封装**

`src/perception/HikCamera.hpp/cpp` 已封装上述流程，对外只暴露：

```cpp
bool init(const HikConfig& cfg = {});  // 枚举+打开+参数设置+开始采图
bool grab(cv::Mat& frame);             // 取一帧，返回 BGR Mat
void setExposure(float us);            // 实时修改曝光
void setGain(float gain);             // 实时修改增益
void close();
```
