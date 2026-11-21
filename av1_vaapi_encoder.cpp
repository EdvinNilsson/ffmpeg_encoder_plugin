#include "av1_vaapi_encoder.h"

#include "hw_encoder_capabilities.h"

VideoEncoderInfo Av1VaapiEncoder::encoderInfo = {
    .UUID{0x95, 0x43, 0x6f, 0x52, 0xfb, 0xab, 0x11, 0x60, 0x04, 0x51, 0xf8, 0x5b, 0x83, 0xa4, 0x76, 0x4d},
    .codecGroup = "AV1",
    .fourCC = 'av01',
    .encoder = "av1_vaapi",
    .hwAcceleration = Vaapi,
    .qualityModes = CQP | VBR,
    .qp = {1, 25, 63},
    .presets = {{FF_COMPRESSION_DEFAULT, "Default"}},
    .defaultPreset = FF_COMPRESSION_DEFAULT,
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

Av1VaapiEncoder::Av1VaapiEncoder(const int formatIndex) : FFmpegEncoder(encoderInfo, formatIndex) {}

StatusCode Av1VaapiEncoder::RegisterCodecs(HostListRef* list) {
    InitializeVaapiPresets(encoderInfo);
    return FFmpegEncoder::RegisterCodecs(list, encoderInfo);
}

StatusCode Av1VaapiEncoder::GetEncoderSettings(HostPropertyCollectionRef* values, HostListRef* settingsList) {
    return FFmpegEncoder::GetEncoderSettings(values, settingsList, encoderInfo);
}
