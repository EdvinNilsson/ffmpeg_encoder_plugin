#pragma once

#include "ffmpeg_encoder.h"

namespace IOPlugin {

class Av1NvencEncoder final : public FFmpegEncoder {
   public:
    static const EncoderInfo encoderInfo;

    Av1NvencEncoder();

    static StatusCode RegisterCodecs(HostListRef* list);
    static StatusCode GetEncoderSettings(HostPropertyCollectionRef* values, HostListRef* settingsList);
};

}
