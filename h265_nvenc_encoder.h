#pragma once

#include "ffmpeg_encoder.h"

namespace IOPlugin {

class H265NvencEncoder final : public FFmpegEncoder {
   public:
    static const EncoderInfo encoderInfo;

    explicit H265NvencEncoder(int formatIndex);

    static StatusCode RegisterCodecs(HostListRef* list);
    static StatusCode GetEncoderSettings(HostPropertyCollectionRef* values, HostListRef* settingsList);
};

}
