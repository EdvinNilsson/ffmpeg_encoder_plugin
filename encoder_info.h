#pragma once
#include <map>
#include <string>
#include <vector>

extern "C" {
#include <libavutil/avutil.h>
}

namespace IOPlugin {

enum HardwareAcceleration { None, Vaapi, Nvenc };

struct EncoderFormat {
    const char* codecName{};
    int bitDepth{8};
    uint32_t colorModel{};
    uint8_t hSubsampling{};
    uint8_t vSubsampling{};
    AVPixelFormat pixelFormat{};
    AVPixelFormat srcPixelFormat{AV_PIX_FMT_NONE};
};

struct EncoderInfo {
    uint8_t UUID[16]{};
    const char* codecGroup{};
    uint32_t fourCC{};
    const char* encoder{};
    HardwareAcceleration hwAcceleration{};
    int32_t qualityModes{};
    uint8_t qp[3]{};
    std::map<int, std::string> presets{};
    int defaultPreset{};
    const char* customParamsKey{};
    std::vector<EncoderFormat> formats{};
};

}
