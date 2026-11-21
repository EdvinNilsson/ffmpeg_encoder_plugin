#include "aac_encoder.h"

const AudioEncoderInfo AACEncoder::encoderInfo = {
    .UUID{0xc7, 0x28, 0x1e, 0xc9, 0x9d, 0x48, 0x67, 0xb7, 0xd5, 0x44, 0x44, 0xbe, 0xe5, 0x63, 0x5d, 0x59},
    .fourCC = 'aac ',
    .encoder = "aac",
    .formats =
        {
            {
                .codecName = "AAC 16-bit (FFmpeg)",
                .bitDepth = 16,
            },
            {
                .codecName = "AAC 24-bit (FFmpeg)",
                .bitDepth = 24,
            },
        },
};

AACEncoder::AACEncoder(const int formatIndex) : AudioEncoder(encoderInfo, formatIndex) {}

StatusCode AACEncoder::RegisterCodecs(HostListRef* list) { return AudioEncoder::RegisterCodecs(list, encoderInfo); }

StatusCode AACEncoder::GetEncoderSettings(HostPropertyCollectionRef* values, HostListRef* settingsList) {
    return AudioEncoder::GetEncoderSettings(values, settingsList, encoderInfo);
}
