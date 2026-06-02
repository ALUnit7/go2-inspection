# 海康工业相机 MVS SDK 接口（C/C++）

> SDK 版本：V4.8.0 | 头文件：`/opt/MVS/include/MvCameraControl.h` | 库：`/opt/MVS/lib/64/libMvCameraControl.so`

---

## 典型采图流程

```
MV_CC_EnumDevices        枚举设备
MV_CC_CreateHandle       创建句柄
MV_CC_OpenDevice         打开设备
MV_CC_SetEnumValue       配置参数（触发模式等）
MV_CC_StartGrabbing      开始采图
MV_CC_GetOneFrameTimeout 取一帧（轮询）
  或 MV_CC_RegisterImageCallBackEx2  注册回调
MV_CC_StopGrabbing       停止采图
MV_CC_CloseDevice        关闭设备
MV_CC_DestroyHandle      销毁句柄
```

---

## 关键接口

### 枚举与连接

```cpp
// 枚举设备（GigE + USB3）
MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &devList);

// 创建/销毁句柄
MV_CC_CreateHandle(&handle, devList.pDeviceInfo[0]);
MV_CC_DestroyHandle(handle);

// 打开/关闭
MV_CC_OpenDevice(handle);   // 默认独占模式
MV_CC_CloseDevice(handle);
```

### 采图

```cpp
// 轮询取帧（推荐，简单）
MV_FRAME_OUT_INFO_EX info{};
MV_CC_GetOneFrameTimeout(handle, buf, bufSize, &info, 1000 /*ms*/);

// 回调取帧（低延迟）
MV_CC_RegisterImageCallBackEx2(handle, callback, pUser);
```

### 参数设置

```cpp
// 关闭触发模式（连续采图）
MV_CC_SetEnumValue(handle, "TriggerMode", 0);

// 读取分辨率
MVCC_INTVALUE val{};
MV_CC_GetIntValue(handle, "Width",  &val);  // val.nCurValue
MV_CC_GetIntValue(handle, "Height", &val);

// 曝光时间（微秒）
MV_CC_SetFloatValue(handle, "ExposureTime", 5000.0f);

// 增益
MV_CC_SetFloatValue(handle, "Gain", 1.0f);
```

### 像素格式转换

```cpp
// Bayer/其他格式 → BGR（用于 OpenCV）
MV_CC_PIXEL_CONVERT_PARAM_EX cvt;
cvt.nWidth         = info.nWidth;
cvt.nHeight        = info.nHeight;
cvt.enSrcPixelType = info.enPixelType;
cvt.pSrcData       = srcBuf;
cvt.nSrcDataLen    = info.nFrameLen;
cvt.enDstPixelType = PixelType_Gvsp_BGR8_Packed;
cvt.pDstBuffer     = dstBuf;
cvt.nDstBufferSize = nWidth * nHeight * 3;
MV_CC_ConvertPixelTypeEx(handle, &cvt);  // 注意用 _EX 版本
```

---

## 常用像素格式

| 枚举值 | 说明 | OpenCV 类型 |
|--------|------|------------|
| `PixelType_Gvsp_Mono8` | 灰度 8bit | `CV_8UC1` |
| `PixelType_Gvsp_RGB8_Packed` | RGB 24bit | `CV_8UC3`（需 RGB→BGR）|
| `PixelType_Gvsp_BGR8_Packed` | BGR 24bit | `CV_8UC3` 直接用 |
| `PixelType_Gvsp_BayerRG8` 等 | Bayer | 需 ConvertPixelTypeEx |

---

## CMake 配置

```cmake
target_include_directories(target PRIVATE /opt/MVS/include)
target_link_libraries(target PRIVATE /opt/MVS/lib/64/libMvCameraControl.so)
```

运行时需要：
```bash
echo "/opt/MVS/lib/64" | sudo tee /etc/ld.so.conf.d/mvs.conf && sudo ldconfig
```

---

## 本项目封装

`src/perception/HikCamera.hpp/cpp` — 封装了完整采图流程，对外只暴露：

```cpp
bool init();              // 枚举并打开第一个相机
bool grab(cv::Mat& frame); // 取一帧转为 BGR Mat
void close();
```
