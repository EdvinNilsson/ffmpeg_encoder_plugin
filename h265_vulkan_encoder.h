#pragma once

#include "ffmpeg_encoder.h"

namespace IOPlugin {

class H265VulkanEncoder final : public FFmpegEncoder {
   public:
    static VideoEncoderInfo encoderInfo;

    explicit H265VulkanEncoder(int formatIndex);
    static StatusCode RegisterCodecs(HostListRef* list);

    static StatusCode GetEncoderSettings(HostPropertyCollectionRef* values, HostListRef* settingsList);
};

}
