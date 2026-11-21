#pragma once

#include <memory>

#include "encoder_info.h"
#include "uisettings_controller.h"
#include "wrapper/plugin_api.h"

extern "C" {
#include <libavcodec/avcodec.h>
}

using namespace IOPlugin;

class AudioEncoder : public IPluginCodecRef
{
protected:
    AudioEncoder(AudioEncoderInfo encoderInfo, int formatIndex);
    ~AudioEncoder() override;

    static StatusCode RegisterCodecs(HostListRef* list, const AudioEncoderInfo& encoderInfo);
    static StatusCode GetEncoderSettings(HostPropertyCollectionRef* values, HostListRef* settingsList,
                                         const AudioEncoderInfo& encoderInfo);


    void DoFlush() override;
    StatusCode DoInit(HostPropertyCollectionRef* p_pProps) override;
    StatusCode DoOpen(HostBufferRef* p_pBuff) override;
    StatusCode DoProcess(HostBufferRef* p_pBuff) override;

    AudioEncoderInfo encoderInfo{};
    int formatIndex{};

private:
    static bool IsEncoderSupported(const AudioEncoderInfo& encoderInfo);

    std::unique_ptr<UISettingsController> settings;

    AVCodecContext* ctx{};
    AVPacket* pkt{};
    AVFrame* frame{};
    int numberOfChannels{};
    uint32_t bitDepth{};
    int bytesPerSample{};
    int frameSampleIndex{};
    uint8_t adtsHeader[7]{};
    int64_t pts{};
};
