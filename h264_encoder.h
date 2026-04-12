#pragma once

#include "ffmpeg_encoder.h"

namespace IOPlugin {

class H264Encoder final : public FFmpegEncoder {
   public:
    static const EncoderInfo encoderInfo;

    explicit H264Encoder(int formatIndex);

    static StatusCode RegisterCodecs(HostListRef* list);
    static StatusCode GetEncoderSettings(HostPropertyCollectionRef* values, HostListRef* settingsList);
};

}
