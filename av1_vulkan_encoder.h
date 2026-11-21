#pragma once

#include "ffmpeg_encoder.h"

namespace IOPlugin {

class Av1VulkanEncoder final : public FFmpegEncoder {
   public:
    static VideoEncoderInfo encoderInfo;

    explicit Av1VulkanEncoder(int formatIndex);
    static StatusCode RegisterCodecs(HostListRef* list);

    static StatusCode GetEncoderSettings(HostPropertyCollectionRef* values, HostListRef* settingsList);
};

}
