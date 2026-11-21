#pragma once

#include "encoder_info.h"
#include "wrapper/plugin_api.h"

namespace IOPlugin {

enum SettingsType {
    VideoSettings,
    AudioSettings,
};

enum QualityMode : int32_t {
    CQP = 1 << 0,
    CRF = 1 << 1,
    VBR = 1 << 2,
};

class UISettingsController final {
   public:
    explicit UISettingsController(VideoEncoderInfo  encoderInfo);
    explicit UISettingsController(AudioEncoderInfo  encoderInfo);
    void Load(IPropertyProvider* values);
    StatusCode Render(HostListRef* settingsList) const;

   private:
    void InitDefaults();
    void SetFirstSupportedQualityMode();
    StatusCode RenderVideoQuality(HostListRef* settingsList) const;
    StatusCode RenderAudioQuality(HostListRef* settingsList) const;

   public:
    QualityMode GetQualityMode() const;
    int32_t GetQP() const;
    int32_t GetBitRate() const;
    int32_t GetPreset() const;
    int32_t GetTune() const;
    const std::string& GetCustomParams() const;

   private:
    SettingsType type;
    VideoEncoderInfo videoEncoderInfo;
    AudioEncoderInfo audioEncoderInfo;

    QualityMode qualityMode{};
    int32_t qp{};
    int32_t bitRate{};
    int32_t preset{};
    int32_t tune{};
    std::string customParams;

    std::string qualityModeId;
    std::string qpId;
    std::string bitrateId;
    std::string presetId;
    std::string tuneId;
    std::string customParamsId;
};

}
