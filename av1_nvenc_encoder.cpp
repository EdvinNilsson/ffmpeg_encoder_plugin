#include "av1_nvenc_encoder.h"

const EncoderInfo Av1NvencEncoder::encoderInfo = {
    .UUID{0x68, 0x78, 0x8d, 0xc5, 0xdd, 0xb1, 0x73, 0x07, 0x46, 0x10, 0xdf, 0xa7, 0xb3, 0x82, 0x89, 0x16},
    .codecGroup = "AV1",
    .fourCC = 'av01',
    .encoder = "av1_nvenc",
    .hwAcceleration = Nvenc,
    .qualityModes = CQP | VBR,
    .qp = {0, 0, 51},
    .presets =
        {
            {0, "p1"},
            {1, "p2"},
            {2, "p3"},
            {3, "p4"},
            {4, "p5"},
            {5, "p6"},
            {6, "p7"},
        },
    .defaultPreset = 3,
    .formats =
        {
            {
                .codecName = "NVENC 8-bit 4:2:0 (FFmpeg)",
                .bitDepth = 8,
                .colorModel = clrNV12,
                .hSubsampling = 2,
                .vSubsampling = 2,
                .pixelFormat = AV_PIX_FMT_NV12,
            },
            {
                .codecName = "NVENC 10-bit 4:2:0 (FFmpeg)",
                .bitDepth = 10,
                .colorModel = clrNV12,
                .hSubsampling = 2,
                .vSubsampling = 2,
                .pixelFormat = AV_PIX_FMT_P010,
            },
        },
};

Av1NvencEncoder::Av1NvencEncoder(const int formatIndex) {
    FFmpegEncoder::encoderInfo = encoderInfo;
    FFmpegEncoder::formatIndex = formatIndex;
}

StatusCode Av1NvencEncoder::RegisterCodecs(HostListRef* list) {
    return FFmpegEncoder::RegisterCodecs(list, encoderInfo);
}

StatusCode Av1NvencEncoder::GetEncoderSettings(HostPropertyCollectionRef* values, HostListRef* settingsList) {
    return FFmpegEncoder::GetEncoderSettings(values, settingsList, encoderInfo);
}
