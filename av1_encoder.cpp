#include "av1_encoder.h"

const EncoderInfo Av1Encoder::encoderInfo = {
    .UUID{0x95, 0x43, 0x6f, 0x52, 0xfb, 0xab, 0x11, 0x60, 0x04, 0x51, 0xf8, 0x5b, 0x83, 0xa4, 0x76, 0x4d},
    .codecGroup = "AV1",
    .codecName = "VAAPI (FFmpeg)",
    .fourCC = 'av01',
    .encoder = "av1_vaapi",
    .hwAcceleration = Vaapi,
    .pixelFormat = AV_PIX_FMT_NV12,
    .colorModel = clrNV12,
    .hSubsampling = 2,
    .vSubsampling = 2,
    .qualityModes = CQP | VBR,
    .qp = {1, 25, 63},
    .presets = {{0, "Speed"}, {1, "Balanced"}, {2, "Quality"}},
    .defaultPreset = 1,
};

Av1Encoder::Av1Encoder() { FFmpegEncoder::encoderInfo = encoderInfo; }

StatusCode Av1Encoder::RegisterCodecs(HostListRef* list) { return FFmpegEncoder::RegisterCodecs(list, encoderInfo); }

StatusCode Av1Encoder::GetEncoderSettings(HostPropertyCollectionRef* values, HostListRef* settingsList) {
    return FFmpegEncoder::GetEncoderSettings(values, settingsList, encoderInfo);
}
