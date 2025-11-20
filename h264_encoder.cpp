#include "h264_encoder.h"

const EncoderInfo H264Encoder::encoderInfo = {
    .UUID{0xaa, 0xeb, 0x0f, 0x28, 0x4c, 0x6f, 0x4e, 0x3f, 0x95, 0xdd, 0x23, 0x43, 0x41, 0xf3, 0x8d, 0xa0},
    .codecGroup = "H.264",
    .codecName = "VAAPI (FFmpeg)",
    .fourCC = 'avc1',
    .encoder = "h264_vaapi",
    .hwAcceleration = Vaapi,
    .pixelFormat = AV_PIX_FMT_NV12,
    .colorModel = clrNV12,
    .hSubsampling = 2,
    .vSubsampling = 2,
    .qualityModes = CQP | VBR,
    .qp = {1, 20, 51},
    .presets = {{0, "Speed"}, {1, "Balanced"}, {2, "Quality"}},
    .defaultPreset = 1,
};

H264Encoder::H264Encoder() { FFmpegEncoder::encoderInfo = encoderInfo; }

StatusCode H264Encoder::RegisterCodecs(HostListRef* list) { return FFmpegEncoder::RegisterCodecs(list, encoderInfo); }

StatusCode H264Encoder::GetEncoderSettings(HostPropertyCollectionRef* values, HostListRef* settingsList) {
    return FFmpegEncoder::GetEncoderSettings(values, settingsList, encoderInfo);
}
