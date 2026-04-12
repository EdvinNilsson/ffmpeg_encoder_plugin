#include "av1_encoder.h"

const EncoderInfo Av1Encoder::encoderInfo = {
    .UUID{0x95, 0x43, 0x6f, 0x52, 0xfb, 0xab, 0x11, 0x60, 0x04, 0x51, 0xf8, 0x5b, 0x83, 0xa4, 0x76, 0x4d},
    .codecGroup = "AV1",
    .fourCC = 'av01',
    .encoder = "av1_vaapi",
    .hwAcceleration = Vaapi,
    .qualityModes = CQP | VBR,
    .qp = {1, 25, 63},
    .presets = {{0, "Speed"}, {1, "Balanced"}, {2, "Quality"}},
    .defaultPreset = 1,
    .formats =
        {
            {
                .codecName = "VAAPI 8-bit 4:2:0 (FFmpeg)",
                .bitDepth = 8,
                .colorModel = clrNV12,
                .hSubsampling = 2,
                .vSubsampling = 2,
                .pixelFormat = AV_PIX_FMT_NV12,
            },
            {
                .codecName = "VAAPI 10-bit 4:2:0 (FFmpeg)",
                .bitDepth = 10,
                .colorModel = clrNV12,
                .hSubsampling = 2,
                .vSubsampling = 2,
                .pixelFormat = AV_PIX_FMT_P010,
            },
        },
};

Av1Encoder::Av1Encoder(const int formatIndex) {
    FFmpegEncoder::encoderInfo = encoderInfo;
    FFmpegEncoder::formatIndex = formatIndex;
}

StatusCode Av1Encoder::RegisterCodecs(HostListRef* list) { return FFmpegEncoder::RegisterCodecs(list, encoderInfo); }

StatusCode Av1Encoder::GetEncoderSettings(HostPropertyCollectionRef* values, HostListRef* settingsList) {
    return FFmpegEncoder::GetEncoderSettings(values, settingsList, encoderInfo);
}
