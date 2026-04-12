#pragma once

#include "ffmpeg_encoder.h"

namespace IOPlugin {

class H264NvencEncoder final : public FFmpegEncoder {
   public:
    static const EncoderInfo encoderInfo;

    explicit H264NvencEncoder(int formatIndex);

    static StatusCode RegisterCodecs(HostListRef* list);
    static StatusCode GetEncoderSettings(HostPropertyCollectionRef* values, HostListRef* settingsList);
};

}
