#pragma once

#include "ffmpeg_encoder.h"

namespace IOPlugin {

class X265Encoder final : public FFmpegEncoder {
   public:
    static const VideoEncoderInfo encoderInfo;

    explicit X265Encoder(int formatIndex);

    static StatusCode RegisterCodecs(HostListRef* list);
    static StatusCode GetEncoderSettings(HostPropertyCollectionRef* values, HostListRef* settingsList);
};

}
