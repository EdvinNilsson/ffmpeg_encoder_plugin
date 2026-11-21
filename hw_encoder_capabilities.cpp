#include "hw_encoder_capabilities.h"

#include <cstring>

#include "host_api.h"

extern "C" {
#include <libavutil/hwcontext.h>

#if HAVE_VA_LIBRARY
#include <libavutil/hwcontext_vaapi.h>
#include <va/va.h>
#endif

#if HAVE_VULKAN_LIBRARY
#include <libavutil/hwcontext_vulkan.h>
#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>
#ifdef __linux__
#include <dlfcn.h>
#endif
#endif
}

#if HAVE_VA_LIBRARY
static const AVVAAPIDeviceContext* GetVaapiContext(const AVBufferRef* hwDeviceCtx) {
    if (!hwDeviceCtx) return nullptr;
    auto* hwCtx = reinterpret_cast<const AVHWDeviceContext*>(hwDeviceCtx->data);
    if (!hwCtx || hwCtx->type != AV_HWDEVICE_TYPE_VAAPI) return nullptr;
    return static_cast<const AVVAAPIDeviceContext*>(hwCtx->hwctx);
}

static bool IsMesaRadeonSi(const AVBufferRef* hwDeviceCtx) {
    const AVVAAPIDeviceContext* vaapi = GetVaapiContext(hwDeviceCtx);
    if (!vaapi || !vaapi->display) return false;

    const char* vendor = vaQueryVendorString(vaapi->display);
    if (!vendor) return false;

    return std::strstr(vendor, "radeonsi") != nullptr;
}

static uint32_t GetVaapiQualityLevelCount(const AVBufferRef* hwDeviceCtx, const VAProfile profile) {
    const AVVAAPIDeviceContext* vaapi = GetVaapiContext(hwDeviceCtx);
    if (!vaapi || !vaapi->display || !profile) return 0;

    VAConfigAttrib attr{};
    attr.type = VAConfigAttribEncQualityRange;

    if (const VAStatus status = vaGetConfigAttributes(vaapi->display, profile, VAEntrypointEncSlice, &attr, 1);
        status != VA_STATUS_SUCCESS)
        return 0;

    if (attr.value == VA_ATTRIB_NOT_SUPPORTED) return 0;

    return attr.value;
}
#endif

void InitializeVaapiPresets(EncoderInfo& encoderInfo) {
#if HAVE_VA_LIBRARY
    AVBufferRef* hwDeviceCtx = nullptr;
    if (av_hwdevice_ctx_create(&hwDeviceCtx, AV_HWDEVICE_TYPE_VAAPI, nullptr, nullptr, 0) < 0) goto end;

    if (IsMesaRadeonSi(hwDeviceCtx)) {
        encoderInfo.presets = {{0, "Speed"}, {1, "Balanced"}, {2, "Quality"}};
        encoderInfo.defaultPreset = 1;
        encoderInfo.isVaapiRadeonSi = true;
    } else {
        VAProfile profile;

        switch (encoderInfo.fourCC) {
            case 'avc1':
                profile = VAProfileH264Main;
                break;
            case 'hvc1':
                profile = VAProfileHEVCMain;
                break;
            case 'av01':
                profile = VAProfileAV1Profile0;
                break;
            default:
                goto end;
        }

        const uint32_t qualityLevels = GetVaapiQualityLevelCount(hwDeviceCtx, profile);
        for (uint32_t i = 1; i <= qualityLevels; ++i) {
            encoderInfo.presets.emplace(i, std::to_string(i));
        }
    }

end:
    av_buffer_unref(&hwDeviceCtx);
#endif
}

#ifdef HAVE_VULKAN_LIBRARY
static AVVulkanDeviceContext* GetVulkanDeviceCtx(const AVBufferRef* hwDeviceCtx) {
    if (!hwDeviceCtx) return nullptr;
    const auto* hwCtx = reinterpret_cast<AVHWDeviceContext*>(hwDeviceCtx->data);
    if (hwCtx->type != AV_HWDEVICE_TYPE_VULKAN) return nullptr;
    return static_cast<AVVulkanDeviceContext*>(hwCtx->hwctx);
}

static uint32_t GetVulkanQualityLevelCount(const AVBufferRef* hwDeviceCtx, const VkVideoProfileInfoKHR* profile,
                                           VkVideoEncodeCapabilitiesKHR* encodeCaps) {
    const AVVulkanDeviceContext* vk = GetVulkanDeviceCtx(hwDeviceCtx);
    if (!vk || !profile) return 0;

    VkVideoCapabilitiesKHR caps{VK_STRUCTURE_TYPE_VIDEO_CAPABILITIES_KHR};
    caps.pNext = encodeCaps;

    const auto vkGetPhysicalDeviceVideoCapabilitiesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceVideoCapabilitiesKHR>(
        vk->get_proc_addr(vk->inst, "vkGetPhysicalDeviceVideoCapabilitiesKHR"));

    if (!vkGetPhysicalDeviceVideoCapabilitiesKHR) return 0;

    if (vkGetPhysicalDeviceVideoCapabilitiesKHR(vk->phys_dev, profile, &caps) != VK_SUCCESS) {
        return 0;
    }

    return encodeCaps->maxQualityLevels;
}
#endif

void InitializeVulkanPresets(EncoderInfo& encoderInfo) {
#ifdef HAVE_VULKAN_LIBRARY

#ifdef __linux__
    // Disable LSFG-VK to prevent a segmentation fault when Vulkan is loaded, due to symbols loaded by LSFG-VK are
    // conflicting with symbols from Resolve.
    setenv("VK_LOADER_LAYERS_DISABLE", "*LS_frame_generation*,*LSFGVK_frame_generation*", 1);
#endif

    AVBufferRef* hwDeviceCtx = nullptr;
    if (av_hwdevice_ctx_create(&hwDeviceCtx, AV_HWDEVICE_TYPE_VULKAN, nullptr, nullptr, 0) < 0) goto end;

    {
        VkVideoProfileInfoKHR profile{VK_STRUCTURE_TYPE_VIDEO_PROFILE_INFO_KHR};
        VkVideoEncodeCapabilitiesKHR encodeCaps{VK_STRUCTURE_TYPE_VIDEO_ENCODE_CAPABILITIES_KHR};

        VkVideoEncodeH264ProfileInfoKHR h264EncodeProfile{VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_PROFILE_INFO_KHR};
        VkVideoEncodeH265ProfileInfoKHR h265EncodeProfile{VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_PROFILE_INFO_KHR};
        VkVideoEncodeAV1ProfileInfoKHR av1EncodeProfile{VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_PROFILE_INFO_KHR};

        VkVideoEncodeH264CapabilitiesKHR h264Caps{VK_STRUCTURE_TYPE_VIDEO_ENCODE_H264_CAPABILITIES_KHR};
        VkVideoEncodeH265CapabilitiesKHR h265Caps{VK_STRUCTURE_TYPE_VIDEO_ENCODE_H265_CAPABILITIES_KHR};
        VkVideoEncodeAV1CapabilitiesKHR av1Caps{VK_STRUCTURE_TYPE_VIDEO_ENCODE_AV1_CAPABILITIES_KHR};

        switch (encoderInfo.fourCC) {
            case 'avc1':
                h264EncodeProfile.stdProfileIdc = STD_VIDEO_H264_PROFILE_IDC_HIGH;
                profile.pNext = &h264EncodeProfile;
                encodeCaps.pNext = &h264Caps;
                profile.videoCodecOperation = VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR;
                break;
            case 'hvc1':
                h265EncodeProfile.stdProfileIdc = STD_VIDEO_H265_PROFILE_IDC_MAIN;
                profile.pNext = &h265EncodeProfile;
                encodeCaps.pNext = &h265Caps;
                profile.videoCodecOperation = VK_VIDEO_CODEC_OPERATION_ENCODE_H265_BIT_KHR;
                break;
            case 'av01':
                av1EncodeProfile.stdProfile = STD_VIDEO_AV1_PROFILE_MAIN;
                profile.pNext = &av1EncodeProfile;
                encodeCaps.pNext = &av1Caps;
                profile.videoCodecOperation = VK_VIDEO_CODEC_OPERATION_ENCODE_AV1_BIT_KHR;
                break;
            default:
                goto end;
        }

        profile.chromaSubsampling = VK_VIDEO_CHROMA_SUBSAMPLING_420_BIT_KHR;
        profile.lumaBitDepth = VK_VIDEO_COMPONENT_BIT_DEPTH_8_BIT_KHR;
        profile.chromaBitDepth = VK_VIDEO_COMPONENT_BIT_DEPTH_8_BIT_KHR;

        const uint32_t qualityLevels = GetVulkanQualityLevelCount(hwDeviceCtx, &profile, &encodeCaps);
        for (uint32_t i = 1; i <= qualityLevels; ++i) {
            encoderInfo.presets.emplace(i, std::to_string(i));
        }
    }

end:
    av_buffer_unref(&hwDeviceCtx);
#endif
}

bool IsVulkanLoaderSupported(const AVBufferRef* hwDeviceCtx) {
#if defined(HAVE_VULKAN_LIBRARY) && defined(__linux__)
    const AVVulkanDeviceContext* vk = GetVulkanDeviceCtx(hwDeviceCtx);
    if (!vk) return false;

    const PFN_vkVoidFunction vkGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR =
        vk->get_proc_addr(vk->inst, "vkGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR");

    if (!vkGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR) return true;

    Dl_info info;
    if (dladdr(reinterpret_cast<void*>(vkGetPhysicalDeviceVideoEncodeQualityLevelPropertiesKHR), &info) &&
        info.dli_sname) {
        // Disable Vulkan to prevent a segmentation fault if the loader generated a logical device trampoline for a
        // physical device function.
        if (const std::string symbol_name(info.dli_sname); symbol_name.find("vkdev_ext") != std::string::npos) {
            g_Log(logLevelWarn, "FFmpeg Plugin :: Vulkan encoders disabled. Please update to a newer Vulkan loader.");
            return false;
        }
    }
#endif
    return true;
}