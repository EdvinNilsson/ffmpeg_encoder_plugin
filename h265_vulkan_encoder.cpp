#include "h265_vulkan_encoder.h"

#include "hw_encoder_capabilities.h"

VideoEncoderInfo H265VulkanEncoder::encoderInfo = {
    .UUID{0x3d, 0x0a, 0x4b, 0x28, 0x49, 0xb7, 0x4f, 0x5b, 0xb9, 0x6a, 0x4e, 0x76, 0x24, 0x21, 0x9d, 0x40},
    .codecGroup = "H.265",
    .fourCC = 'hvc1',
    .encoder = "hevc_vulkan",
    .hwAcceleration = Vulkan,
    .qualityModes = CQP | VBR,
    .qp = {1, 25, 51},
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

H265VulkanEncoder::H265VulkanEncoder(const int formatIndex) : FFmpegEncoder(encoderInfo, formatIndex) {}

StatusCode H265VulkanEncoder::RegisterCodecs(HostListRef* list) {
    InitializeVulkanPresets(encoderInfo);
    return FFmpegEncoder::RegisterCodecs(list, encoderInfo);
}

StatusCode H265VulkanEncoder::GetEncoderSettings(HostPropertyCollectionRef* values, HostListRef* settingsList) {
    return FFmpegEncoder::GetEncoderSettings(values, settingsList, encoderInfo);
}
