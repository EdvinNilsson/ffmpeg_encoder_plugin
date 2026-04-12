#include "h265_encoder.h"

const EncoderInfo H265Encoder::encoderInfo = {
    .UUID{0x2b, 0xfa, 0xeb, 0xed, 0xaa, 0x3d, 0x45, 0xbf, 0xe2, 0x27, 0x32, 0x4a, 0x96, 0x7a, 0xb4, 0x66},
    .codecGroup = "H.265",
    .fourCC = 'hvc1',
    .encoder = "hevc_vaapi",
    .hwAcceleration = Vaapi,
    .qualityModes = CQP | VBR,
    .qp = {1, 25, 51},
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

H265Encoder::H265Encoder(const int formatIndex) {
    FFmpegEncoder::encoderInfo = encoderInfo;
    FFmpegEncoder::formatIndex = formatIndex;
}

StatusCode H265Encoder::RegisterCodecs(HostListRef* list) { return FFmpegEncoder::RegisterCodecs(list, encoderInfo); }

StatusCode H265Encoder::GetEncoderSettings(HostPropertyCollectionRef* values, HostListRef* settingsList) {
    return FFmpegEncoder::GetEncoderSettings(values, settingsList, encoderInfo);
}
