#include "h265_encoder.h"

const EncoderInfo H265Encoder::encoderInfo = {
    .UUID{0x2b, 0xfa, 0xeb, 0xed, 0xaa, 0x3d, 0x45, 0xbf, 0xe2, 0x27, 0x32, 0x4a, 0x96, 0x7a, 0xb4, 0x66},
    .codecGroup = "H.265",
    .codecName = "VAAPI (FFmpeg)",
    .fourCC = 'hvc1',
    .encoder = "hevc_vaapi",
    .hwAcceleration = Vaapi,
    .pixelFormat = AV_PIX_FMT_NV12,
    .colorModel = clrNV12,
    .hSubsampling = 2,
    .vSubsampling = 2,
    .qualityModes = CQP | VBR,
    .qp = {1, 25, 51},
    .presets = {{0, "Speed"}, {1, "Balanced"}, {2, "Quality"}},
    .defaultPreset = 1,
};

H265Encoder::H265Encoder() { FFmpegEncoder::encoderInfo = encoderInfo; }

StatusCode H265Encoder::RegisterCodecs(HostListRef* list) { return FFmpegEncoder::RegisterCodecs(list, encoderInfo); }

StatusCode H265Encoder::GetEncoderSettings(HostPropertyCollectionRef* values, HostListRef* settingsList) {
    return FFmpegEncoder::GetEncoderSettings(values, settingsList, encoderInfo);
}
