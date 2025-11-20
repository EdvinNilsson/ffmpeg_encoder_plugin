#include "h265_nvenc_encoder.h"

const EncoderInfo H265NvencEncoder::encoderInfo = {
    .UUID{0xe6, 0x85, 0xbb, 0x38, 0xb5, 0xf4, 0x72, 0xe9, 0x2a, 0x6a, 0x77, 0x2f, 0xe6, 0x7f, 0x3e, 0x8e},
    .codecGroup = "H.265",
    .codecName = "NVENC (FFmpeg)",
    .fourCC = 'hvc1',
    .encoder = "hevc_nvenc",
    .hwAcceleration = Nvenc,
    .pixelFormat = AV_PIX_FMT_NV12,
    .colorModel = clrNV12,
    .hSubsampling = 2,
    .vSubsampling = 2,
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
};

H265NvencEncoder::H265NvencEncoder() { FFmpegEncoder::encoderInfo = encoderInfo; }

StatusCode H265NvencEncoder::RegisterCodecs(HostListRef* list) {
    return FFmpegEncoder::RegisterCodecs(list, encoderInfo);
}

StatusCode H265NvencEncoder::GetEncoderSettings(HostPropertyCollectionRef* values, HostListRef* settingsList) {
    return FFmpegEncoder::GetEncoderSettings(values, settingsList, encoderInfo);
}
