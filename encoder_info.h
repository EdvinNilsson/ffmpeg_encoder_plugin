#pragma once
#include <map>
#include <string>

extern "C" {
#include <libavutil/avutil.h>
}

namespace IOPlugin {

enum HardwareAcceleration { None, Vaapi, Nvenc };

struct EncoderInfo {
    uint8_t UUID[16]{};
    const char* codecGroup{};
    const char* codecName{};
    uint32_t fourCC{};
    const char* encoder{};
    HardwareAcceleration hwAcceleration{};
    AVPixelFormat pixelFormat{};
    uint32_t colorModel{};
    uint8_t hSubsampling{};
    uint8_t vSubsampling{};
    int32_t qualityModes{};
    uint8_t qp[3]{};
    std::map<int, std::string> presets{};
    int defaultPreset{};
};

}
