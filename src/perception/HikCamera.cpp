#include "HikCamera.hpp"
#include <opencv2/imgproc.hpp>
#include <iostream>

bool HikCamera::init(const HikConfig& cfg) {
    MV_CC_DEVICE_INFO_LIST devList{};
    if (MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &devList) != MV_OK
        || devList.nDeviceNum == 0) {
        std::cerr << "[HikCamera] No device found\n";
        return false;
    }
    if (MV_CC_CreateHandle(&handle_, devList.pDeviceInfo[0]) != MV_OK ||
        MV_CC_OpenDevice(handle_) != MV_OK) {
        std::cerr << "[HikCamera] Open failed\n";
        return false;
    }

    // 连续采图模式
    MV_CC_SetEnumValue(handle_, "TriggerMode", 0);

    // 曝光
    if (cfg.exposure_us > 0) {
        MV_CC_SetEnumValue(handle_, "ExposureAuto", 0);  // 关闭自动曝光
        MV_CC_SetFloatValue(handle_, "ExposureTime", cfg.exposure_us);
    } else {
        MV_CC_SetEnumValue(handle_, "ExposureAuto", 2);  // 连续自动曝光
    }

    // 增益
    if (cfg.gain >= 0) {
        MV_CC_SetEnumValue(handle_, "GainAuto", 0);      // 关闭自动增益
        MV_CC_SetFloatValue(handle_, "Gain", cfg.gain);
    } else {
        MV_CC_SetEnumValue(handle_, "GainAuto", 2);      // 连续自动增益
    }

    // 白平衡
    MV_CC_SetEnumValue(handle_, "BalanceWhiteAuto", cfg.auto_wb ? 2 : 0);

    // 读取分辨率
    MVCC_INTVALUE val{};
    MV_CC_GetIntValue(handle_, "Width",  &val); width_  = val.nCurValue;
    MV_CC_GetIntValue(handle_, "Height", &val); height_ = val.nCurValue;
    buf_.resize(width_ * height_ * 3);

    if (MV_CC_StartGrabbing(handle_) != MV_OK) {
        std::cerr << "[HikCamera] StartGrabbing failed\n";
        return false;
    }
    std::cout << "[HikCamera] " << width_ << "x" << height_
              << " exp=" << (cfg.exposure_us > 0 ? std::to_string((int)cfg.exposure_us)+"us" : "auto")
              << " gain=" << (cfg.gain >= 0 ? std::to_string(cfg.gain) : "auto") << "\n";
    return true;
}

bool HikCamera::grab(cv::Mat& frame) {
    MV_FRAME_OUT_INFO_EX info{};
    int ret = MV_CC_GetOneFrameTimeout(handle_, buf_.data(),
                                       (unsigned int)buf_.size(), &info, 1000);
    if (ret != MV_OK) {
        std::cerr << "[HikCamera] GetFrame error=0x" << std::hex << ret << "\n";
        return false;
    }

    if (info.enPixelType == PixelType_Gvsp_Mono8) {
        cv::Mat mono(info.nHeight, info.nWidth, CV_8UC1, buf_.data());
        cv::cvtColor(mono, frame, cv::COLOR_GRAY2BGR);
    } else if (info.enPixelType == PixelType_Gvsp_RGB8_Packed) {
        cv::Mat rgb(info.nHeight, info.nWidth, CV_8UC3, buf_.data());
        cv::cvtColor(rgb, frame, cv::COLOR_RGB2BGR);
    } else if (info.enPixelType == PixelType_Gvsp_BGR8_Packed) {
        frame = cv::Mat(info.nHeight, info.nWidth, CV_8UC3, buf_.data()).clone();
    } else {
        std::vector<unsigned char> dst(info.nWidth * info.nHeight * 3);
        MV_CC_PIXEL_CONVERT_PARAM_EX cvt{};
        cvt.nWidth         = info.nWidth;
        cvt.nHeight        = info.nHeight;
        cvt.enSrcPixelType = info.enPixelType;
        cvt.pSrcData       = buf_.data();
        cvt.nSrcDataLen    = info.nFrameLen;
        cvt.enDstPixelType = PixelType_Gvsp_BGR8_Packed;
        cvt.pDstBuffer     = dst.data();
        cvt.nDstBufferSize = (unsigned int)dst.size();
        if (MV_CC_ConvertPixelTypeEx(handle_, &cvt) != MV_OK) return false;
        frame = cv::Mat(info.nHeight, info.nWidth, CV_8UC3, dst.data()).clone();
    }
    return !frame.empty();
}

void HikCamera::setExposure(float us) {
    if (us > 0) {
        MV_CC_SetEnumValue(handle_, "ExposureAuto", 0);
        MV_CC_SetFloatValue(handle_, "ExposureTime", us);
    } else {
        MV_CC_SetEnumValue(handle_, "ExposureAuto", 2);
    }
}

void HikCamera::setGain(float gain) {
    if (gain >= 0) {
        MV_CC_SetEnumValue(handle_, "GainAuto", 0);
        MV_CC_SetFloatValue(handle_, "Gain", gain);
    } else {
        MV_CC_SetEnumValue(handle_, "GainAuto", 2);
    }
}

void HikCamera::close() {
    if (!handle_) return;
    MV_CC_StopGrabbing(handle_);
    MV_CC_CloseDevice(handle_);
    MV_CC_DestroyHandle(handle_);
    handle_ = nullptr;
}
