#pragma once
#include "encoder_info.h"

using namespace IOPlugin;

void InitializeVaapiPresets(VideoEncoderInfo& encoderInfo);
void InitializeVulkanPresets(VideoEncoderInfo& encoderInfo);

bool IsVulkanLoaderSupported(const AVBufferRef* hwDeviceCtx);
