#include "h264_nvenc_encoder.h"

const EncoderInfo H264NvencEncoder::encoderInfo = {
    .UUID{0x10, 0x2f, 0x9d, 0x71, 0x19, 0xd7, 0xa8, 0xde, 0x90, 0xb1, 0xff, 0x35, 0x91, 0x4f, 0xaa, 0x36},
    .codecGroup = "H.264",
    .codecName = "NVENC (FFmpeg)",
    .fourCC = 'avc1',
    .encoder = "h264_nvenc",
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

H264NvencEncoder::H264NvencEncoder() { FFmpegEncoder::encoderInfo = encoderInfo; }

StatusCode H264NvencEncoder::RegisterCodecs(HostListRef* list) {
    return FFmpegEncoder::RegisterCodecs(list, encoderInfo);
}

StatusCode H264NvencEncoder::GetEncoderSettings(HostPropertyCollectionRef* values, HostListRef* settingsList) {
    return FFmpegEncoder::GetEncoderSettings(values, settingsList, encoderInfo);
}
