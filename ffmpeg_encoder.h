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
    FFmpegEncoder(VideoEncoderInfo encoderInfo, int formatIndex);
    ~FFmpegEncoder() override;

    StatusCode DoInit(HostPropertyCollectionRef* p_pProps) override;
    static StatusCode RegisterCodecs(HostListRef* list, const VideoEncoderInfo& encoderInfo);
    static StatusCode GetEncoderSettings(HostPropertyCollectionRef* values, HostListRef* settingsList,
                                         const VideoEncoderInfo& encoderInfo);
    StatusCode DoOpen(HostBufferRef* p_pBuff) override;
    virtual StatusCode ApplyOptions(AVCodecContext* ctx, UISettingsController& settings, HostBufferRef* p_pBuff);
    StatusCode DoProcess(HostBufferRef* p_pBuff) override;
    void DoFlush() override;

    VideoEncoderInfo encoderInfo{};
    int formatIndex{};
    HostCodecConfigCommon commonProps{};

   private:
    static bool IsEncoderSupported(const VideoEncoderInfo& encoderInfo, int formatIndex);

    std::unique_ptr<UISettingsController> settings{};

    int height{};
    int width{};
    uint32_t frameRateDen{};
    uint32_t frameRateNum{};
    AVPixelFormat pixelFormat{};
    AVPixelFormat srcPixelFormat{};
    bool useHwDevice{};
    PixelFormatRepacker* pixelFormatConverter{};

    AVCodecContext* ctx{};
    AVBufferRef* hwDeviceCtx{};
    AVPacket* pkt{};
    AVFrame* swFrame{};
};

}
