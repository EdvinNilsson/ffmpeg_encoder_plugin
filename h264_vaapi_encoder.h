#pragma once

#include "ffmpeg_encoder.h"

namespace IOPlugin {

class H264VaapiEncoder final : public FFmpegEncoder {
   public:
    static VideoEncoderInfo encoderInfo;

    explicit H264VaapiEncoder(int formatIndex);

    static StatusCode RegisterCodecs(HostListRef* list);
    static StatusCode GetEncoderSettings(HostPropertyCollectionRef* values, HostListRef* settingsList);
};

}
