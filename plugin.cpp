#include "plugin.h"

#include <cstring>

#include "aac_encoder.h"
#include "av1_nvenc_encoder.h"
#include "av1_vaapi_encoder.h"
#include "av1_vulkan_encoder.h"
#include "h264_nvenc_encoder.h"
#include "h264_vaapi_encoder.h"
#include "h264_vulkan_encoder.h"
#include "h265_nvenc_encoder.h"
#include "h265_vaapi_encoder.h"
#include "h265_vulkan_encoder.h"
#include "svt_av1_encoder.h"
#include "x264_encoder.h"
#include "x265_encoder.h"

static const uint8_t UUID[] = {0xdc, 0xe2, 0xaf, 0xa7, 0x91, 0xe0, 0x75, 0xb7,
                               0x29, 0x41, 0xe8, 0xe0, 0xee, 0xa2, 0x52, 0x59};

StatusCode g_HandleGetInfo(HostPropertyCollectionRef* p_pProps) {
    StatusCode err = p_pProps->SetProperty(pIOPropUUID, propTypeUInt8, UUID, 16);
    if (err == errNone) {
        const char* name = "FFmpeg Plugin";
        err = p_pProps->SetProperty(pIOPropName, propTypeString, name, static_cast<int>(strlen(name)));
    }

    return err;
}

StatusCode g_HandleCreateObj(unsigned char* p_pUUID, ObjectRef* p_ppObj) {
    if (memcmp(p_pUUID, H264VaapiEncoder::encoderInfo.UUID, 15) == 0) {
        const uint8_t formatIndex = p_pUUID[15] - H264VaapiEncoder::encoderInfo.UUID[15];
        *p_ppObj = new H264VaapiEncoder(formatIndex);
        return errNone;
    }

    if (memcmp(p_pUUID, H264NvencEncoder::encoderInfo.UUID, 15) == 0) {
        const uint8_t formatIndex = p_pUUID[15] - H264NvencEncoder::encoderInfo.UUID[15];
        *p_ppObj = new H264NvencEncoder(formatIndex);
        return errNone;
    }

    if (memcmp(p_pUUID, H264VulkanEncoder::encoderInfo.UUID, 15) == 0) {
        const uint8_t formatIndex = p_pUUID[15] - H264VulkanEncoder::encoderInfo.UUID[15];
        *p_ppObj = new H264VulkanEncoder(formatIndex);
        return errNone;
    }

    if (memcmp(p_pUUID, X264Encoder::encoderInfo.UUID, 15) == 0) {
        const uint8_t formatIndex = p_pUUID[15] - X264Encoder::encoderInfo.UUID[15];
        *p_ppObj = new X264Encoder(formatIndex);
        return errNone;
    }

    if (memcmp(p_pUUID, H265VaapiEncoder::encoderInfo.UUID, 15) == 0) {
        const uint8_t formatIndex = p_pUUID[15] - H265VaapiEncoder::encoderInfo.UUID[15];
        *p_ppObj = new H265VaapiEncoder(formatIndex);
        return errNone;
    }

    if (memcmp(p_pUUID, H265NvencEncoder::encoderInfo.UUID, 15) == 0) {
        const uint8_t formatIndex = p_pUUID[15] - H265NvencEncoder::encoderInfo.UUID[15];
        *p_ppObj = new H265NvencEncoder(formatIndex);
        return errNone;
    }

    if (memcmp(p_pUUID, H265VulkanEncoder::encoderInfo.UUID, 15) == 0) {
        const uint8_t formatIndex = p_pUUID[15] - H265VulkanEncoder::encoderInfo.UUID[15];
        *p_ppObj = new H265VulkanEncoder(formatIndex);
        return errNone;
    }

    if (memcmp(p_pUUID, X265Encoder::encoderInfo.UUID, 15) == 0) {
        const uint8_t formatIndex = p_pUUID[15] - X265Encoder::encoderInfo.UUID[15];
        *p_ppObj = new X265Encoder(formatIndex);
        return errNone;
    }

    if (memcmp(p_pUUID, Av1VaapiEncoder::encoderInfo.UUID, 15) == 0) {
        const uint8_t formatIndex = p_pUUID[15] - Av1VaapiEncoder::encoderInfo.UUID[15];
        *p_ppObj = new Av1VaapiEncoder(formatIndex);
        return errNone;
    }

    if (memcmp(p_pUUID, Av1NvencEncoder::encoderInfo.UUID, 15) == 0) {
        const uint8_t formatIndex = p_pUUID[15] - Av1NvencEncoder::encoderInfo.UUID[15];
        *p_ppObj = new Av1NvencEncoder(formatIndex);
        return errNone;
    }

    if (memcmp(p_pUUID, Av1VulkanEncoder::encoderInfo.UUID, 15) == 0) {
        const uint8_t formatIndex = p_pUUID[15] - Av1VulkanEncoder::encoderInfo.UUID[15];
        *p_ppObj = new Av1VulkanEncoder(formatIndex);
        return errNone;
    }

    if (memcmp(p_pUUID, SvtAv1Encoder::encoderInfo.UUID, 15) == 0) {
        const uint8_t formatIndex = p_pUUID[15] - SvtAv1Encoder::encoderInfo.UUID[15];
        *p_ppObj = new SvtAv1Encoder(formatIndex);
        return errNone;
    }

    if (memcmp(p_pUUID, AACEncoder::encoderInfo.UUID, 15) == 0) {
        const uint8_t formatIndex = p_pUUID[15] - AACEncoder::encoderInfo.UUID[15];
        *p_ppObj = new AACEncoder(formatIndex);
        return errNone;
    }

    return errUnsupported;
}

StatusCode g_HandlePluginStart() { return errNone; }

StatusCode g_HandlePluginTerminate() { return errNone; }

StatusCode g_ListCodecs(HostListRef* p_pList) {
    StatusCode err = H264VaapiEncoder::RegisterCodecs(p_pList);
    if (err != errNone) return err;

    err = H264NvencEncoder::RegisterCodecs(p_pList);
    if (err != errNone) return err;

    err = H264VulkanEncoder::RegisterCodecs(p_pList);
    if (err != errNone) return err;

    err = X264Encoder::RegisterCodecs(p_pList);
    if (err != errNone) return err;

    err = H265VaapiEncoder::RegisterCodecs(p_pList);
    if (err != errNone) return err;

    err = H265NvencEncoder::RegisterCodecs(p_pList);
    if (err != errNone) return err;

    err = H265VulkanEncoder::RegisterCodecs(p_pList);
    if (err != errNone) return err;

    err = X265Encoder::RegisterCodecs(p_pList);
    if (err != errNone) return err;

    err = Av1VaapiEncoder::RegisterCodecs(p_pList);
    if (err != errNone) return err;

    err = Av1NvencEncoder::RegisterCodecs(p_pList);
    if (err != errNone) return err;

    err = Av1VulkanEncoder::RegisterCodecs(p_pList);
    if (err != errNone) return err;

    err = SvtAv1Encoder::RegisterCodecs(p_pList);
    if (err != errNone) return err;

    err = AACEncoder::RegisterCodecs(p_pList);
    if (err != errNone) return err;

    return errNone;
}

StatusCode g_ListContainers(HostListRef* p_pList) { return errNone; }

StatusCode g_GetEncoderSettings(unsigned char* p_pUUID, HostPropertyCollectionRef* p_pValues,
                                HostListRef* p_pSettingsList) {
    if (memcmp(p_pUUID, H264VaapiEncoder::encoderInfo.UUID, 15) == 0) {
        return H264VaapiEncoder::GetEncoderSettings(p_pValues, p_pSettingsList);
    }

    if (memcmp(p_pUUID, H264NvencEncoder::encoderInfo.UUID, 15) == 0) {
        return H264NvencEncoder::GetEncoderSettings(p_pValues, p_pSettingsList);
    }

    if (memcmp(p_pUUID, H264VulkanEncoder::encoderInfo.UUID, 15) == 0) {
        return H264VulkanEncoder::GetEncoderSettings(p_pValues, p_pSettingsList);
    }

    if (memcmp(p_pUUID, X264Encoder::encoderInfo.UUID, 15) == 0) {
        return X264Encoder::GetEncoderSettings(p_pValues, p_pSettingsList);
    }

    if (memcmp(p_pUUID, H265VaapiEncoder::encoderInfo.UUID, 15) == 0) {
        return H265VaapiEncoder::GetEncoderSettings(p_pValues, p_pSettingsList);
    }

    if (memcmp(p_pUUID, H265NvencEncoder::encoderInfo.UUID, 15) == 0) {
        return H265NvencEncoder::GetEncoderSettings(p_pValues, p_pSettingsList);
    }

    if (memcmp(p_pUUID, H265VulkanEncoder::encoderInfo.UUID, 15) == 0) {
        return H265VulkanEncoder::GetEncoderSettings(p_pValues, p_pSettingsList);
    }

    if (memcmp(p_pUUID, X265Encoder::encoderInfo.UUID, 15) == 0) {
        return X265Encoder::GetEncoderSettings(p_pValues, p_pSettingsList);
    }

    if (memcmp(p_pUUID, Av1VaapiEncoder::encoderInfo.UUID, 15) == 0) {
        return Av1VaapiEncoder::GetEncoderSettings(p_pValues, p_pSettingsList);
    }

    if (memcmp(p_pUUID, Av1NvencEncoder::encoderInfo.UUID, 15) == 0) {
        return Av1NvencEncoder::GetEncoderSettings(p_pValues, p_pSettingsList);
    }

    if (memcmp(p_pUUID, Av1VulkanEncoder::encoderInfo.UUID, 15) == 0) {
        return Av1VulkanEncoder::GetEncoderSettings(p_pValues, p_pSettingsList);
    }

    if (memcmp(p_pUUID, SvtAv1Encoder::encoderInfo.UUID, 15) == 0) {
        return SvtAv1Encoder::GetEncoderSettings(p_pValues, p_pSettingsList);
    }

    if (memcmp(p_pUUID, AACEncoder::encoderInfo.UUID, 15) == 0) {
        return AACEncoder::GetEncoderSettings(p_pValues, p_pSettingsList);
    }

    return errNoCodec;
}
