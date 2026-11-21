#pragma once

#include "ffmpeg_encoder.h"

namespace IOPlugin {

class H265VaapiEncoder final : public FFmpegEncoder {
   public:
    static VideoEncoderInfo encoderInfo;

    explicit H265VaapiEncoder(int formatIndex);

    static StatusCode RegisterCodecs(HostListRef* list);
    static StatusCode GetEncoderSettings(HostPropertyCollectionRef* values, HostListRef* settingsList);
};

}
