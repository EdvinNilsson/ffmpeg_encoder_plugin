#pragma once
#include <map>
#include <string>
#include <vector>

#include "pixel_format_repacker.h"

namespace IOPlugin {

enum HardwareAcceleration { None, Vaapi, Nvenc, Vulkan };

struct VideoEncoderFormat {
    const char* codecName{};
    int bitDepth{8};
    uint32_t colorModel{};
    uint8_t hSubsampling{};
    uint8_t vSubsampling{};
    AVPixelFormat pixelFormat{};
    AVPixelFormat srcPixelFormat{AV_PIX_FMT_NONE};
    PixelFormatRepacker* pixelFormatRepacker{};
};

struct VideoEncoderInfo {
    uint8_t UUID[16]{};
    const char* codecGroup{};
    uint32_t fourCC{};
    const char* encoder{};
    HardwareAcceleration hwAcceleration{};
    int32_t qualityModes{};
    uint8_t qp[3]{};
    std::map<int, std::string> presets{};
    int defaultPreset{};
    std::map<int, std::string> tunes{};
    int defaultTune{-1};
    const char* customParamsKey{};
    std::vector<VideoEncoderFormat> formats{};
    bool isVaapiRadeonSi{};
};

}
