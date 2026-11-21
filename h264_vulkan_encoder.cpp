#include "h264_vulkan_encoder.h"

#include "hw_encoder_capabilities.h"

VideoEncoderInfo H264VulkanEncoder::encoderInfo = {
    .UUID{0x12, 0x1c, 0x5e, 0x0b, 0x16, 0xb2, 0x44, 0x26, 0xbd, 0x97, 0x83, 0x17, 0x91, 0x52, 0x69, 0xe8},
    .codecGroup = "H.264",
    .fourCC = 'avc1',
    .encoder = "h264_vulkan",
    .hwAcceleration = Vulkan,
    .qualityModes = CQP | VBR,
    .qp = {1, 20, 51},
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

H264VulkanEncoder::H264VulkanEncoder(const int formatIndex) : FFmpegEncoder(encoderInfo, formatIndex) {}

StatusCode H264VulkanEncoder::RegisterCodecs(HostListRef* list) {
    InitializeVulkanPresets(encoderInfo);
    return FFmpegEncoder::RegisterCodecs(list, encoderInfo);
}

StatusCode H264VulkanEncoder::GetEncoderSettings(HostPropertyCollectionRef* values, HostListRef* settingsList) {
    return FFmpegEncoder::GetEncoderSettings(values, settingsList, encoderInfo);
}
