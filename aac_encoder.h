#pragma once

#include "audio_encoder.h"
#include "encoder_info.h"

namespace IOPlugin {

class AACEncoder final : public AudioEncoder {
   public:
    static const AudioEncoderInfo encoderInfo;

    explicit AACEncoder(int formatIndex);

    static StatusCode RegisterCodecs(HostListRef* list);
    static StatusCode GetEncoderSettings(HostPropertyCollectionRef* values, HostListRef* settingsList);
};

}
