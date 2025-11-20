#pragma once

#include "ffmpeg_encoder.h"

namespace IOPlugin {

class H265NvencEncoder final : public FFmpegEncoder {
   public:
    static const EncoderInfo encoderInfo;

    H265NvencEncoder();

    static StatusCode RegisterCodecs(HostListRef* list);
    static StatusCode GetEncoderSettings(HostPropertyCollectionRef* values, HostListRef* settingsList);
};

}
