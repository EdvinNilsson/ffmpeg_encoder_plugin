#include "ffmpeg_encoder.h"

UISettingsController::UISettingsController(const EncoderInfo& encoderInfo) : encoderInfo(encoderInfo) {
    InitDefaults();
}

UISettingsController::UISettingsController(const HostCodecConfigCommon& commonProps, const EncoderInfo& encoderInfo)
    : commonProps(commonProps), encoderInfo(encoderInfo) {
    InitDefaults();
}

void UISettingsController::Load(IPropertyProvider* values) {
    uint8_t val8 = 0;
    values->GetUINT8("ffmpeg_reset", val8);
    if (val8 != 0) {
        *this = UISettingsController(encoderInfo);
        return;
    }

    int32_t val32 = 0;
    values->GetINT32(qualityModeId.c_str(), val32);
    qualityMode = static_cast<QualityMode>(val32);

    if (!(qualityMode & encoderInfo.qualityModes)) {
        if (encoderInfo.qualityModes & CRF)
            qualityMode = CRF;
        else if (encoderInfo.qualityModes & CQP)
            qualityMode = CQP;
        else if (encoderInfo.qualityModes & VBR)
            qualityMode = VBR;
    }

    values->GetINT32(qpId.c_str(), qp);
    values->GetINT32(bitrateId.c_str(), bitRate);
    values->GetINT32(presetId.c_str(), preset);
}

StatusCode UISettingsController::Render(HostListRef* settingsList) const {
    StatusCode err = RenderQuality(settingsList);
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
    qualityMode = CRF;
    qp = encoderInfo.qp[1];
    bitRate = 6000;
    preset = encoderInfo.defaultPreset;

    const std::string prefix = std::string("ffmpeg_") + encoderInfo.encoder + "_";
    qualityModeId = prefix + "q_mode";
    qpId = prefix + "qp";
    bitrateId = prefix + "bitrate";
    presetId = prefix + "preset";
}

StatusCode UISettingsController::RenderQuality(HostListRef* settingsList) const {
    {
        HostUIConfigEntryRef item(presetId);

        std::vector<std::string> textsVec;
        std::vector<int> valuesVec;

        for (const auto& [key, value] : encoderInfo.presets) {
            valuesVec.push_back(key);
            textsVec.emplace_back(value);
        }

        item.MakeComboBox("Encoder Preset", textsVec, valuesVec, preset);
        if (!item.IsSuccess() || !settingsList->Append(&item)) {
            g_Log(logLevelError, "FFmpeg Plugin :: Failed to populate encoder preset UI entry");
            return errFail;
        }
    }

    {
        HostUIConfigEntryRef item(qualityModeId);

        std::vector<std::string> textsVec;
        std::vector<int> valuesVec;

        if (encoderInfo.qualityModes & CRF) {
            textsVec.emplace_back("Constant Rate Factor");
            valuesVec.push_back(CRF);
        }

        if (encoderInfo.qualityModes & CQP) {
            textsVec.emplace_back("Constant Quality");
            valuesVec.push_back(CQP);
        }

        if (encoderInfo.qualityModes & VBR) {
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
        if (encoderInfo.hwAcceleration == Nvenc && qp == 0) {
            pLabel = "(automatic)";
        } else if (qp < encoderInfo.qp[2] / 3) {
            pLabel = "(high)";
        } else if (qp < encoderInfo.qp[2] * 2 / 3) {
            pLabel = "(medium)";
        } else {
            pLabel = "(low)";
        }
        item.MakeSlider("Factor", pLabel, qp, encoderInfo.qp[0], encoderInfo.qp[2], encoderInfo.qp[1]);
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

    return errNone;
}

QualityMode UISettingsController::GetQualityMode() const { return qualityMode; }

int32_t UISettingsController::GetQP() const { return std::max<int>(0, qp); }

int32_t UISettingsController::GetBitRate() const { return bitRate * 1000; }

int32_t UISettingsController::GetPreset() const { return preset; }
