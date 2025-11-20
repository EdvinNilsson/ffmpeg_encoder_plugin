#include "svt_av1_encoder.h"

const EncoderInfo SvtAv1Encoder::encoderInfo = {
    .UUID{0x97, 0x39, 0xbb, 0x53, 0x9b, 0x97, 0xd0, 0x0e, 0x3e, 0x1c, 0x90, 0xeb, 0x70, 0x75, 0x9d, 0xcc},
    .codecGroup = "AV1",
    .codecName = "SVT-AV1 (FFmpeg)",
    .fourCC = 'av01',
    .encoder = "libsvtav1",
    .hwAcceleration = None,
    .pixelFormat = AV_PIX_FMT_YUV420P,
    .colorModel = clrYUVp,
    .hSubsampling = 2,
    .vSubsampling = 2,
    .qualityModes = CRF | VBR,
    .qp = {1, 35, 63},
    .presets =
        {
            {0, "0"},
            {1, "1"},
            {2, "2"},
            {3, "3"},
            {4, "4"},
            {5, "5"},
            {6, "6"},
            {7, "7"},
            {8, "8"},
            {9, "9"},
            {10, "10"},
            {11, "11"},
            {12, "12"},
        },
    .defaultPreset = 8,
};

SvtAv1Encoder::SvtAv1Encoder() { FFmpegEncoder::encoderInfo = encoderInfo; }

StatusCode SvtAv1Encoder::RegisterCodecs(HostListRef* list) { return FFmpegEncoder::RegisterCodecs(list, encoderInfo); }

StatusCode SvtAv1Encoder::GetEncoderSettings(HostPropertyCollectionRef* values, HostListRef* settingsList) {
    return FFmpegEncoder::GetEncoderSettings(values, settingsList, encoderInfo);
}
