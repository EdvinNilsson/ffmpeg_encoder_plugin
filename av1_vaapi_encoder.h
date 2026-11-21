#pragma once

#include "ffmpeg_encoder.h"

namespace IOPlugin {

class Av1VaapiEncoder final : public FFmpegEncoder {
   public:
    static VideoEncoderInfo encoderInfo;

    explicit Av1VaapiEncoder(int formatIndex);

    static StatusCode RegisterCodecs(HostListRef* list);
    static StatusCode GetEncoderSettings(HostPropertyCollectionRef* values, HostListRef* settingsList);
};

}