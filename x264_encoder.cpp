#include "x264_encoder.h"

const EncoderInfo X264Encoder::encoderInfo = {
    .UUID{0x10, 0x33, 0x0b, 0xa7, 0x8e, 0xf2, 0x9c, 0xc6, 0x19, 0xa3, 0x56, 0xa7, 0x8a, 0xa5, 0x17, 0xc8},
    .codecGroup = "H.264",
    .codecName = "X264 (FFmpeg)",
    .fourCC = 'avc1',
    .encoder = "libx264",
    .hwAcceleration = None,
    .pixelFormat = AV_PIX_FMT_NV12,
    .colorModel = clrNV12,
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

X264Encoder::X264Encoder() { FFmpegEncoder::encoderInfo = encoderInfo; }

StatusCode X264Encoder::RegisterCodecs(HostListRef* list) { return FFmpegEncoder::RegisterCodecs(list, encoderInfo); }

StatusCode X264Encoder::GetEncoderSettings(HostPropertyCollectionRef* values, HostListRef* settingsList) {
    return FFmpegEncoder::GetEncoderSettings(values, settingsList, encoderInfo);
}
