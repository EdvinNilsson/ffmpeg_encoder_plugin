#include "av1_vulkan_encoder.h"

#include "hw_encoder_capabilities.h"

VideoEncoderInfo Av1VulkanEncoder::encoderInfo = {
    .UUID{0x12, 0x1f, 0xa5, 0x6f, 0x3e, 0x4e, 0x4c, 0x3f, 0x9f, 0xf1, 0xb2, 0x26, 0x28, 0xd3, 0xcc, 0x47},
    .codecGroup = "AV1",
    .fourCC = 'av01',
    .encoder = "av1_vulkan",
    .hwAcceleration = Vulkan,
    .qualityModes = CQP | VBR,
    .qp = {1, 25, 63},
    .presets = {{0, "Default"}},
    .defaultPreset = 0,
    .formats =
        {
            {
                .codecName = "Vulkan 8-bit 4:2:0 (FFmpeg)",
                .bitDepth = 8,
                .colorModel = clrNV12,
                .hSubsampling = 2,
                .vSubsampling = 2,
                .pixelFormat = AV_PIX_FMT_NV12,
            },
            {
                .codecName = "Vulkan 10-bit 4:2:0 (FFmpeg)",
                .bitDepth = 10,
                .colorModel = clrNV12,
                .hSubsampling = 2,
                .vSubsampling = 2,
                .pixelFormat = AV_PIX_FMT_P010,
            },
        },
};

Av1VulkanEncoder::Av1VulkanEncoder(const int formatIndex) : FFmpegEncoder(encoderInfo, formatIndex) {}

StatusCode Av1VulkanEncoder::RegisterCodecs(HostListRef* list) {
    InitializeVulkanPresets(encoderInfo);
    return FFmpegEncoder::RegisterCodecs(list, encoderInfo);
}

StatusCode Av1VulkanEncoder::GetEncoderSettings(HostPropertyCollectionRef* values, HostListRef* settingsList) {
    return FFmpegEncoder::GetEncoderSettings(values, settingsList, encoderInfo);
}
