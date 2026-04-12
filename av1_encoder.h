#pragma once

#include "ffmpeg_encoder.h"

namespace IOPlugin {

class Av1Encoder final : public FFmpegEncoder {
   public:
    static const EncoderInfo encoderInfo;

    explicit Av1Encoder(int formatIndex);

    static StatusCode RegisterCodecs(HostListRef* list);
    static StatusCode GetEncoderSettings(HostPropertyCollectionRef* values, HostListRef* settingsList);
};

}