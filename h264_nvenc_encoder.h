#pragma once

#include "ffmpeg_encoder.h"

namespace IOPlugin {

class H264NvencEncoder final : public FFmpegEncoder {
   public:
    static const EncoderInfo encoderInfo;

    H264NvencEncoder();

    static StatusCode RegisterCodecs(HostListRef* list);
    static StatusCode GetEncoderSettings(HostPropertyCollectionRef* values, HostListRef* settingsList);
};

}
