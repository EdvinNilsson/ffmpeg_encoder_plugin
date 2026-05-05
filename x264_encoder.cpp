#include "x264_encoder.h"

const EncoderInfo X264Encoder::encoderInfo = {
    .UUID{0x10, 0x33, 0x0b, 0xa7, 0x8e, 0xf2, 0x9c, 0xc6, 0x19, 0xa3, 0x56, 0xa7, 0x8a, 0xa5, 0x17, 0xc8},
    .codecGroup = "H.264",
    .fourCC = 'avc1',
    .encoder = "libx264",
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
    .customParamsKey = "x264-params",
    .formats =
        {
            {
                .codecName = "x264 8-bit 4:2:0 (FFmpeg)",
                .bitDepth = 8,
                .colorModel = clrNV12,
                .hSubsampling = 2,
                .vSubsampling = 2,
                .pixelFormat = AV_PIX_FMT_NV12,
            },
            {
                .codecName = "x264 10-bit 4:2:0 (FFmpeg)",
                .bitDepth = 10,
                .colorModel = clrNV12,
                .hSubsampling = 2,
                .vSubsampling = 2,
                .pixelFormat = AV_PIX_FMT_YUV420P10LE,
                .srcPixelFormat = AV_PIX_FMT_P010,
            },
            {
                .codecName = "x264 8-bit 4:2:2 (FFmpeg)",
                .bitDepth = 8,
                .colorModel = clrUYVY,
                .hSubsampling = 2,
                .vSubsampling = 1,
                .pixelFormat = AV_PIX_FMT_YUV422P,
                .srcPixelFormat = AV_PIX_FMT_UYVY422,
            },
            {
                .codecName = "x264 10-bit 4:2:2 (FFmpeg)",
                .bitDepth = 16,
                .colorModel = clrYUVp,
                .hSubsampling = 2,
                .vSubsampling = 1,
                .pixelFormat = AV_PIX_FMT_YUV422P10LE,
                .srcPixelFormat = AV_PIX_FMT_YUV422P16LE,
            },
            {
                .codecName = "x264 8-bit 4:4:4 (FFmpeg)",
                .bitDepth = 8,
                .colorModel = clrYUVp,
                .hSubsampling = 1,
                .vSubsampling = 1,
                .pixelFormat = AV_PIX_FMT_YUV444P,
            },
            {
                .codecName = "x264 10-bit 4:4:4 (FFmpeg)",
                .bitDepth = 16,
                .colorModel = clrYUVp,
                .hSubsampling = 1,
                .vSubsampling = 1,
                .pixelFormat = AV_PIX_FMT_YUV444P10LE,
                .srcPixelFormat = AV_PIX_FMT_YUV444P16LE,
            },
        },
};

X264Encoder::X264Encoder(const int formatIndex) {
    FFmpegEncoder::encoderInfo = encoderInfo;
    FFmpegEncoder::formatIndex = formatIndex;
}

StatusCode X264Encoder::RegisterCodecs(HostListRef* list) { return FFmpegEncoder::RegisterCodecs(list, encoderInfo); }

StatusCode X264Encoder::GetEncoderSettings(HostPropertyCollectionRef* values, HostListRef* settingsList) {
    return FFmpegEncoder::GetEncoderSettings(values, settingsList, encoderInfo);
}
