#include "plugin.h"

#include <cstring>

#include "av1_encoder.h"
#include "av1_nvenc_encoder.h"
#include "ffmpeg_encoder.h"
#include "h264_encoder.h"
#include "h264_nvenc_encoder.h"
#include "h265_encoder.h"
#include "h265_nvenc_encoder.h"
#include "svt_av1_encoder.h"
#include "uisettings_controller.h"
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
    if (memcmp(p_pUUID, H264Encoder::encoderInfo.UUID, 16) == 0) {
        *p_ppObj = new H264Encoder();
        return errNone;
    }

    if (memcmp(p_pUUID, X264Encoder::encoderInfo.UUID, 16) == 0) {
        *p_ppObj = new X264Encoder();
        return errNone;
    }

    if (memcmp(p_pUUID, H264NvencEncoder::encoderInfo.UUID, 16) == 0) {
        *p_ppObj = new H264NvencEncoder();
        return errNone;
    }

    if (memcmp(p_pUUID, H265Encoder::encoderInfo.UUID, 16) == 0) {
        *p_ppObj = new H265Encoder();
        return errNone;
    }

    if (memcmp(p_pUUID, X265Encoder::encoderInfo.UUID, 16) == 0) {
        *p_ppObj = new X265Encoder();
        return errNone;
    }

    if (memcmp(p_pUUID, H265NvencEncoder::encoderInfo.UUID, 16) == 0) {
        *p_ppObj = new H265NvencEncoder();
        return errNone;
    }

    if (memcmp(p_pUUID, Av1Encoder::encoderInfo.UUID, 16) == 0) {
        *p_ppObj = new Av1Encoder();
        return errNone;
    }

    if (memcmp(p_pUUID, SvtAv1Encoder::encoderInfo.UUID, 16) == 0) {
        *p_ppObj = new SvtAv1Encoder();
        return errNone;
    }

    if (memcmp(p_pUUID, Av1NvencEncoder::encoderInfo.UUID, 16) == 0) {
        *p_ppObj = new Av1NvencEncoder();
        return errNone;
    }

    return errUnsupported;
}

StatusCode g_HandlePluginStart() { return errNone; }

StatusCode g_HandlePluginTerminate() { return errNone; }

StatusCode g_ListCodecs(HostListRef* p_pList) {
    StatusCode err = H264Encoder::RegisterCodecs(p_pList);
    if (err != errNone) return err;

    err = X264Encoder::RegisterCodecs(p_pList);
    if (err != errNone) return err;

    err = H264NvencEncoder::RegisterCodecs(p_pList);
    if (err != errNone) return err;

    err = H265Encoder::RegisterCodecs(p_pList);
    if (err != errNone) return err;

    err = X265Encoder::RegisterCodecs(p_pList);
    if (err != errNone) return err;

    err = H265NvencEncoder::RegisterCodecs(p_pList);
    if (err != errNone) return err;

    err = Av1Encoder::RegisterCodecs(p_pList);
    if (err != errNone) return err;

    err = SvtAv1Encoder::RegisterCodecs(p_pList);
    if (err != errNone) return err;

    err = Av1NvencEncoder::RegisterCodecs(p_pList);
    if (err != errNone) return err;

    return errNone;
}

StatusCode g_ListContainers(HostListRef* p_pList) { return errNone; }

StatusCode g_GetEncoderSettings(unsigned char* p_pUUID, HostPropertyCollectionRef* p_pValues,
                                HostListRef* p_pSettingsList) {
    if (memcmp(p_pUUID, H264Encoder::encoderInfo.UUID, 16) == 0) {
        return H264Encoder::GetEncoderSettings(p_pValues, p_pSettingsList);
    }

    if (memcmp(p_pUUID, X264Encoder::encoderInfo.UUID, 16) == 0) {
        return X264Encoder::GetEncoderSettings(p_pValues, p_pSettingsList);
    }

    if (memcmp(p_pUUID, H264NvencEncoder::encoderInfo.UUID, 16) == 0) {
        return H264NvencEncoder::GetEncoderSettings(p_pValues, p_pSettingsList);
    }

    if (memcmp(p_pUUID, H265Encoder::encoderInfo.UUID, 16) == 0) {
        return H265Encoder::GetEncoderSettings(p_pValues, p_pSettingsList);
    }

    if (memcmp(p_pUUID, X265Encoder::encoderInfo.UUID, 16) == 0) {
        return X265Encoder::GetEncoderSettings(p_pValues, p_pSettingsList);
    }

    if (memcmp(p_pUUID, H265NvencEncoder::encoderInfo.UUID, 16) == 0) {
        return H265NvencEncoder::GetEncoderSettings(p_pValues, p_pSettingsList);
    }

    if (memcmp(p_pUUID, Av1Encoder::encoderInfo.UUID, 16) == 0) {
        return Av1Encoder::GetEncoderSettings(p_pValues, p_pSettingsList);
    }

    if (memcmp(p_pUUID, SvtAv1Encoder::encoderInfo.UUID, 16) == 0) {
        return SvtAv1Encoder::GetEncoderSettings(p_pValues, p_pSettingsList);
    }

    if (memcmp(p_pUUID, Av1NvencEncoder::encoderInfo.UUID, 16) == 0) {
        return Av1NvencEncoder::GetEncoderSettings(p_pValues, p_pSettingsList);
    }

    return errNoCodec;
}
