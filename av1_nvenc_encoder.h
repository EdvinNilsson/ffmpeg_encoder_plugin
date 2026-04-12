#pragma once

#include "ffmpeg_encoder.h"

namespace IOPlugin {

class Av1NvencEncoder final : public FFmpegEncoder {
   public:
    static const EncoderInfo encoderInfo;

    explicit Av1NvencEncoder(int formatIndex);

    static StatusCode RegisterCodecs(HostListRef* list);
    static StatusCode GetEncoderSettings(HostPropertyCollectionRef* values, HostListRef* settingsList);
};

}
