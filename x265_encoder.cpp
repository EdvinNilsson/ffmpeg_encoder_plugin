#include "x265_encoder.h"

const EncoderInfo X265Encoder::encoderInfo = {
    .UUID{0x45, 0x0f, 0xed, 0x9a, 0x6e, 0x44, 0xec, 0x0f, 0xec, 0x97, 0xe6, 0xb4, 0xe5, 0x53, 0x8f, 0x73},
    .codecGroup = "H.265",
    .fourCC = 'hvc1',
    .encoder = "libx265",
    .hwAcceleration = None,
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
    .customParamsKey = "x265-params",
    .formats =
        {
            {
                .codecName = "x265 8-bit 4:2:0 (FFmpeg)",
                .bitDepth = 8,
                .colorModel = clrYUVp,
                .hSubsampling = 2,
                .vSubsampling = 2,
                .pixelFormat = AV_PIX_FMT_YUV420P,
            },
            {
                .codecName = "x265 10-bit 4:2:0 (FFmpeg)",
                .bitDepth = 10,
                .colorModel = clrNV12,
                .hSubsampling = 2,
                .vSubsampling = 2,
                .pixelFormat = AV_PIX_FMT_YUV420P10LE,
                .srcPixelFormat = AV_PIX_FMT_P010,
            },
            {
                .codecName = "x265 8-bit 4:2:2 (FFmpeg)",
                .bitDepth = 8,
                .colorModel = clrUYVY,
                .hSubsampling = 2,
                .vSubsampling = 1,
                .pixelFormat = AV_PIX_FMT_YUV422P,
                .srcPixelFormat = AV_PIX_FMT_UYVY422,
            },
            {
                .codecName = "x265 10-bit 4:2:2 (FFmpeg)",
                .bitDepth = 16,
                .colorModel = clrYUVp,
                .hSubsampling = 2,
                .vSubsampling = 1,
                .pixelFormat = AV_PIX_FMT_YUV422P10LE,
                .srcPixelFormat = AV_PIX_FMT_YUV422P16LE,
            },
            {
                .codecName = "x265 8-bit 4:4:4 (FFmpeg)",
                .bitDepth = 8,
                .colorModel = clrYUVp,
                .hSubsampling = 1,
                .vSubsampling = 1,
                .pixelFormat = AV_PIX_FMT_YUV444P,
            },
            {
                .codecName = "x265 10-bit 4:4:4 (FFmpeg)",
                .bitDepth = 16,
                .colorModel = clrYUVp,
                .hSubsampling = 1,
                .vSubsampling = 1,
                .pixelFormat = AV_PIX_FMT_YUV444P10LE,
                .srcPixelFormat = AV_PIX_FMT_YUV444P16LE,
            },
        },
};

X265Encoder::X265Encoder(const int formatIndex) {
    FFmpegEncoder::encoderInfo = encoderInfo;
    FFmpegEncoder::formatIndex = formatIndex;
}

StatusCode X265Encoder::RegisterCodecs(HostListRef* list) { return FFmpegEncoder::RegisterCodecs(list, encoderInfo); }

StatusCode X265Encoder::GetEncoderSettings(HostPropertyCollectionRef* values, HostListRef* settingsList) {
    return FFmpegEncoder::GetEncoderSettings(values, settingsList, encoderInfo);
}
