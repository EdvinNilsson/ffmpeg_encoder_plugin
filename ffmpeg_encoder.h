#pragma once

#include <memory>

#include "encoder_info.h"
#include "uisettings_controller.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
}

namespace IOPlugin {

class FFmpegEncoder : public IPluginCodecRef {
   protected:
    FFmpegEncoder();
    ~FFmpegEncoder() override;

    StatusCode DoInit(HostPropertyCollectionRef* p_pProps) override;
    static StatusCode RegisterCodecs(HostListRef* list, const EncoderInfo& encoderInfo);
    static StatusCode GetEncoderSettings(HostPropertyCollectionRef* values, HostListRef* settingsList,
                                         const EncoderInfo& encoderInfo);
    StatusCode DoOpen(HostBufferRef* p_pBuff) override;
    virtual void ApplyOptions(AVCodecContext* ctx, UISettingsController& settings);
    StatusCode DoProcess(HostBufferRef* p_pBuff) override;
    void DoFlush() override;

    EncoderInfo encoderInfo{};
    HostCodecConfigCommon commonProps{};

   private:
    static bool IsEncoderSupported(const EncoderInfo& encoderInfo);

    std::unique_ptr<UISettingsController> settings{};

    uint32_t height{};
    uint32_t width{};
    uint32_t frameRateDen{};
    uint32_t frameRateNum{};
    AVPixelFormat pixelFormat{};
    bool useVaapi{};

    AVCodecContext* ctx{};
    AVBufferRef* hwDeviceCtx{};
    AVPacket* pkt{};
    AVFrame* swFrame{};
};

}
