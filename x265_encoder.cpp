#include "x265_encoder.h"

const EncoderInfo X265Encoder::encoderInfo = {
    .UUID{0x45, 0x0f, 0xed, 0x9a, 0x6e, 0x44, 0xec, 0x0f, 0xec, 0x97, 0xe6, 0xb4, 0xe5, 0x53, 0x8f, 0x73},
    .codecGroup = "H.265",
    .codecName = "X265 (FFmpeg)",
    .fourCC = 'hvc1',
    .encoder = "libx265",
    .hwAcceleration = None,
    .pixelFormat = AV_PIX_FMT_YUV420P,
    .colorModel = clrYUVp,
    .hSubsampling = 2,
    .vSubsampling = 2,
    .qualityModes = CQP | CRF | VBR,
    .qp = {0, 23, 51},
    .presets =
        {
            {0, "ultrafast"},
            {1, "superfast"},
            {2, "veryfast"},
            {3, "faster"},
            {4, "fast"},
            {5, "medium"},
            {6, "slow"},
            {7, "slower"},
            {8, "veryslow"},
        },
    .defaultPreset = 5,
};

X265Encoder::X265Encoder() { FFmpegEncoder::encoderInfo = encoderInfo; }

StatusCode X265Encoder::RegisterCodecs(HostListRef* list) { return FFmpegEncoder::RegisterCodecs(list, encoderInfo); }

StatusCode X265Encoder::GetEncoderSettings(HostPropertyCollectionRef* values, HostListRef* settingsList) {
    return FFmpegEncoder::GetEncoderSettings(values, settingsList, encoderInfo);
}
