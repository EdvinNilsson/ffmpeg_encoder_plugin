#include "uisettings_controller.h"

#include <utility>

UISettingsController::UISettingsController(VideoEncoderInfo encoderInfo) : videoEncoderInfo(std::move(encoderInfo)) {
    type = VideoSettings;
    InitDefaults();
}


void UISettingsController::Load(IPropertyProvider* values) {
    uint8_t val8 = 0;
    values->GetUINT8("ffmpeg_reset", val8);
    if (val8 != 0) {
        switch (type) {
            case VideoSettings:
                *this = UISettingsController(videoEncoderInfo);
                return;
            default:
                return;
        }
    }

    int32_t val32 = 0;
    values->GetINT32(qualityModeId.c_str(), val32);
    qualityMode = static_cast<QualityMode>(val32);
    SetFirstSupportedQualityMode();

    values->GetINT32(qpId.c_str(), qp);
    values->GetINT32(bitrateId.c_str(), bitRate);
    values->GetINT32(presetId.c_str(), preset);
    values->GetINT32(tuneId.c_str(), tune);

    std::string customParamsStr;
    if (values->GetString(customParamsId.c_str(), customParamsStr)) {
        customParams = customParamsStr;
    }
}

StatusCode UISettingsController::Render(HostListRef* settingsList) const {
    StatusCode err = errNone;
    switch (type) {
        case VideoSettings:
            err = RenderVideoQuality(settingsList);
            break;
    }

    if (err != errNone) {
        return err;
    }

    {
        HostUIConfigEntryRef item("ffmpeg_reset");
        item.MakeButton("Reset");
        item.SetTriggersUpdate(true);
        if (!item.IsSuccess() || !settingsList->Append(&item)) {
            g_Log(logLevelError, "FFmpeg Plugin :: Failed to populate the button entry");
            return errFail;
        }
    }

    return errNone;
}

void UISettingsController::InitDefaults() {
    const char* encoder = nullptr;

    switch (type) {
        case VideoSettings:
            encoder = videoEncoderInfo.encoder;

            qualityMode = CRF;
            SetFirstSupportedQualityMode();

            qp = videoEncoderInfo.qp[1];
            bitRate = 6000;
            preset = videoEncoderInfo.defaultPreset;
            tune = videoEncoderInfo.defaultTune;
            break;

    }

    const std::string prefix = std::string("ffmpeg_") + encoder + "_";
    qualityModeId = prefix + "q_mode";
    qpId = prefix + "qp";
    bitrateId = prefix + "bitrate";
    presetId = prefix + "preset";
    tuneId = prefix + "tune";
    customParamsId = prefix + "custom_params";
}

void UISettingsController::SetFirstSupportedQualityMode() {
    if (!(qualityMode & videoEncoderInfo.qualityModes)) {
        if (videoEncoderInfo.qualityModes & CRF)
            qualityMode = CRF;
        else if (videoEncoderInfo.qualityModes & CQP)
            qualityMode = CQP;
        else if (videoEncoderInfo.qualityModes & VBR)
            qualityMode = VBR;
    }
}

StatusCode UISettingsController::RenderVideoQuality(HostListRef* settingsList) const {
    {
        HostUIConfigEntryRef item(presetId);

        std::vector<std::string> textsVec;
        std::vector<int> valuesVec;

        for (const auto& [key, value] : videoEncoderInfo.presets) {
            valuesVec.push_back(key);
            textsVec.emplace_back(value);
        }

        item.MakeComboBox("Encoder Preset", textsVec, valuesVec, preset);
        if (!item.IsSuccess() || !settingsList->Append(&item)) {
            g_Log(logLevelError, "FFmpeg Plugin :: Failed to populate encoder preset UI entry");
            return errFail;
        }
    }

    if (!videoEncoderInfo.tunes.empty()) {
        HostUIConfigEntryRef item(tuneId);

        std::vector<std::string> textsVec;
        std::vector<int> valuesVec;

        for (const auto& [key, value] : videoEncoderInfo.tunes) {
            valuesVec.push_back(key);
            textsVec.emplace_back(value);
        }

        item.MakeComboBox("Encoder Tune", textsVec, valuesVec, tune);
        if (!item.IsSuccess() || !settingsList->Append(&item)) {
            g_Log(logLevelError, "FFmpeg Plugin :: Failed to populate encoder tune UI entry");
            return errFail;
        }
    }

    {
        HostUIConfigEntryRef item(qualityModeId);

        std::vector<std::string> textsVec;
        std::vector<int> valuesVec;

        if (videoEncoderInfo.qualityModes & CRF) {
            textsVec.emplace_back("Constant Rate Factor");
            valuesVec.push_back(CRF);
        }

        if (videoEncoderInfo.qualityModes & CQP) {
            textsVec.emplace_back("Constant Quality");
            valuesVec.push_back(CQP);
        }

        if (videoEncoderInfo.qualityModes & VBR) {
            textsVec.emplace_back("Variable Rate");
            valuesVec.push_back(VBR);
        }

        item.MakeRadioBox("Quality Control", textsVec, valuesVec, qualityMode);
        item.SetTriggersUpdate(true);

        if (!item.IsSuccess() || !settingsList->Append(&item)) {
            g_Log(logLevelError, "FFmpeg Plugin :: Failed to populate quality UI entry");
            return errFail;
        }
    }

    {
        HostUIConfigEntryRef item(qpId);
        const char* pLabel = nullptr;
        if (videoEncoderInfo.hwAcceleration == Nvenc && qp == 0) {
            pLabel = "(automatic)";
        } else if (qp < videoEncoderInfo.qp[2] / 3) {
            pLabel = "(high)";
        } else if (qp < videoEncoderInfo.qp[2] * 2 / 3) {
            pLabel = "(medium)";
        } else {
            pLabel = "(low)";
        }
        item.MakeSlider("Factor", pLabel, qp, videoEncoderInfo.qp[0], videoEncoderInfo.qp[2], videoEncoderInfo.qp[1]);
        item.SetTriggersUpdate(true);
        item.SetHidden(qualityMode == VBR);
        if (!item.IsSuccess() || !settingsList->Append(&item)) {
            g_Log(logLevelError, "FFmpeg Plugin :: Failed to populate qp slider UI entry");
            return errFail;
        }
    }

    {
        HostUIConfigEntryRef item(bitrateId);
        item.MakeSlider("Bit Rate", "kb/s", bitRate, 100, 100000, 1);
        item.SetHidden(qualityMode != VBR);

        if (!item.IsSuccess() || !settingsList->Append(&item)) {
            g_Log(logLevelError, "FFmpeg Plugin :: Failed to populate bitrate slider UI entry");
            return errFail;
        }
    }

    if (videoEncoderInfo.customParamsKey != nullptr) {
        {
            HostUIConfigEntryRef item("separator");
            item.MakeSeparator();
            if (!item.IsSuccess() || !settingsList->Append(&item)) {
                g_Log(logLevelError, "FFmpeg Plugin :: Failed to populate separator");
                return errFail;
            }
        }

        HostUIConfigEntryRef item(customParamsId);
        item.MakeTextBox("Encoder Params", customParams, "");
        if (!item.IsSuccess() || !settingsList->Append(&item)) {
            g_Log(logLevelError, "FFmpeg Plugin :: Failed to populate custom params UI entry");
            return errFail;
        }
    }

    return errNone;
}

QualityMode UISettingsController::GetQualityMode() const { return qualityMode; }

int32_t UISettingsController::GetQP() const { return std::max<int>(0, qp); }

int32_t UISettingsController::GetBitRate() const { return bitRate * 1000; }

int32_t UISettingsController::GetPreset() const { return preset; }

int32_t UISettingsController::GetTune() const { return tune; }

const std::string& UISettingsController::GetCustomParams() const { return customParams; }
