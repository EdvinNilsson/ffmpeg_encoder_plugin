#include "ffmpeg_encoder.h"

extern "C" {
#include <libavutil/opt.h>
}

#undef av_err2str
av_always_inline std::string av_err2string(int errnum) {
    char str[AV_ERROR_MAX_STRING_SIZE];
    return av_make_error_string(str, AV_ERROR_MAX_STRING_SIZE, errnum);
}
#define av_err2str(err) av_err2string(err).c_str()

StatusCode FFmpegEncoder::DoInit(HostPropertyCollectionRef* p_pProps) { return errNone; }

StatusCode FFmpegEncoder::RegisterCodecs(HostListRef* list, const EncoderInfo& encoderInfo) {
    HostPropertyCollectionRef codecInfo;
    if (!codecInfo.IsValid()) {
        return errAlloc;
    }

    if (!IsEncoderSupported(encoderInfo)) {
        g_Log(logLevelWarn, "FFmpeg Plugin :: Encoder '%s' is not supported", encoderInfo.encoder);
        return errNone;
    }

    codecInfo.SetProperty(pIOPropUUID, propTypeUInt8, encoderInfo.UUID, 16);

    const char* pCodecGroup = encoderInfo.codecGroup;
    codecInfo.SetProperty(pIOPropGroup, propTypeString, pCodecGroup, static_cast<int>(strlen(pCodecGroup)));

    const char* pCodecName = encoderInfo.codecName;
    codecInfo.SetProperty(pIOPropName, propTypeString, pCodecName, static_cast<int>(strlen(pCodecName)));

    codecInfo.SetProperty(pIOPropFourCC, propTypeUInt32, &encoderInfo.fourCC, 1);

    uint32_t vMediaVideo = mediaVideo;
    codecInfo.SetProperty(pIOPropMediaType, propTypeUInt32, &vMediaVideo, 1);

    uint32_t vDirection = dirEncode;
    codecInfo.SetProperty(pIOPropCodecDirection, propTypeUInt32, &vDirection, 1);

    uint32_t vColorModel = encoderInfo.colorModel;
    codecInfo.SetProperty(pIOPropColorModel, propTypeUInt32, &vColorModel, 1);
    codecInfo.SetProperty(pIOPropHSubsampling, propTypeUInt8, &encoderInfo.hSubsampling, 1);
    codecInfo.SetProperty(pIOPropVSubsampling, propTypeUInt8, &encoderInfo.vSubsampling, 1);

    std::vector<uint8_t> dataRangeVec;
    dataRangeVec.push_back(0);
    dataRangeVec.push_back(1);
    codecInfo.SetProperty(pIOPropDataRange, propTypeUInt8, dataRangeVec.data(), static_cast<int>(dataRangeVec.size()));

    int val = 8;
    codecInfo.SetProperty(pIOPropBitDepth, propTypeUInt32, &val, 1);
    codecInfo.SetProperty(pIOPropBitsPerSample, propTypeUInt32, &val, 1);

    uint32_t temp = 0;
    codecInfo.SetProperty(pIOPropTemporalReordering, propTypeUInt32, &temp, 1);

    uint8_t fieldSupport = fieldProgressive | fieldTop | fieldBottom;
    codecInfo.SetProperty(pIOPropFieldOrder, propTypeUInt8, &fieldSupport, 1);

    uint8_t threadSafe = 1;
    codecInfo.SetProperty(pIOPropThreadSafe, propTypeUInt8, &threadSafe, 1);

    bool hwAcc = encoderInfo.hwAcceleration != None;
    codecInfo.SetProperty(pIOPropHWAcc, propTypeUInt8, &hwAcc, 1);

    const std::vector<std::string> containerVec = {"mov", "mp4"};
    std::string valStrings;
    for (size_t i = 0; i < containerVec.size(); ++i) {
        valStrings.append(containerVec[i]);
        if (i < containerVec.size() - 1) {
            valStrings.append(1, '\0');
        }
    }

    codecInfo.SetProperty(pIOPropContainerList, propTypeString, valStrings.c_str(),
                          static_cast<int>(valStrings.size()));

    if (!list->Append(&codecInfo)) {
        return errFail;
    }

    return errNone;
}

StatusCode FFmpegEncoder::GetEncoderSettings(HostPropertyCollectionRef* values, HostListRef* settingsList,
                                             const EncoderInfo& encoderInfo) {
    HostCodecConfigCommon commonProps;
    commonProps.Load(values);

    UISettingsController settings(commonProps, encoderInfo);
    settings.Load(values);

    return settings.Render(settingsList);
}

StatusCode FFmpegEncoder::DoOpen(HostBufferRef* p_pBuff) {
    commonProps.Load(p_pBuff);

    settings = std::make_unique<UISettingsController>(commonProps, encoderInfo);
    settings->Load(p_pBuff);

    int16_t colorMatrix = 1;
    int16_t colorPrimaries = 1;
    int16_t transferFunction = 1;

    p_pBuff->SetProperty(pIOColorMatrix, propTypeInt16, &colorMatrix, 1);
    p_pBuff->SetProperty(pIOPropColorPrimaries, propTypeInt16, &colorPrimaries, 1);
    p_pBuff->SetProperty(pIOTransferCharacteristics, propTypeInt16, &transferFunction, 1);

    uint8_t isMultiPass = 0;
    p_pBuff->SetProperty(pIOPropMultiPass, propTypeUInt8, &isMultiPass, 1);

    width = commonProps.GetWidth();
    height = commonProps.GetHeight();
    frameRateNum = commonProps.GetFrameRateNum();
    frameRateDen = commonProps.GetFrameRateDen();
    pixelFormat = encoderInfo.pixelFormat;
    useVaapi = encoderInfo.hwAcceleration == Vaapi;

    const AVCodec* codec = avcodec_find_encoder_by_name(encoderInfo.encoder);
    if (!codec) {
        g_Log(logLevelError, "FFmpeg Plugin :: Encoder '%s' not found", encoderInfo.encoder);
        return errNoCodec;
    }

    ctx = avcodec_alloc_context3(codec);
    if (!ctx) {
        g_Log(logLevelError, "FFmpeg Plugin :: Failed to allocate codec context");
        return errFail;
    }

    ctx->pix_fmt = useVaapi ? AV_PIX_FMT_VAAPI : pixelFormat;
    ctx->width = static_cast<int>(width);
    ctx->height = static_cast<int>(height);
    ctx->time_base = {static_cast<int>(frameRateDen), static_cast<int>(frameRateNum)};
    ctx->framerate = {static_cast<int>(frameRateNum), static_cast<int>(frameRateDen)};
    ctx->thread_count = 0;
    ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    ApplyOptions(ctx, *settings);

    if (useVaapi) {
        int err = av_hwdevice_ctx_create(&hwDeviceCtx, AV_HWDEVICE_TYPE_VAAPI, nullptr, nullptr, 0);

        if (err < 0) {
            g_Log(logLevelError, "FFmpeg Plugin :: Failed to create a VAAPI device. %s", av_err2str(err));
            return errUnsupported;
        }

        AVBufferRef* hwFramesRef;
        AVHWFramesContext* framesCtx = nullptr;

        if (!((hwFramesRef = av_hwframe_ctx_alloc(hwDeviceCtx)))) {
            g_Log(logLevelError, "FFmpeg Plugin :: Failed to create VAAPI frame context");
            av_buffer_unref(&hwFramesRef);
            return errUnsupported;
        }
        framesCtx = reinterpret_cast<AVHWFramesContext*>(hwFramesRef->data);
        framesCtx->format = AV_PIX_FMT_VAAPI;
        framesCtx->sw_format = pixelFormat;
        framesCtx->width = static_cast<int>(width);
        framesCtx->height = static_cast<int>(height);
        framesCtx->initial_pool_size = 20;
        if ((err = av_hwframe_ctx_init(hwFramesRef)) < 0) {
            g_Log(logLevelError, "FFmpeg Plugin :: Failed to initialize VAAPI frame context. %s", av_err2str(err));
            av_buffer_unref(&hwFramesRef);
            return errUnsupported;
        }
        ctx->hw_frames_ctx = av_buffer_ref(hwFramesRef);
        if (!ctx->hw_frames_ctx) {
            av_buffer_unref(&hwFramesRef);
            return errUnsupported;
        }

        av_buffer_unref(&hwFramesRef);
    }

    if (avcodec_open2(ctx, codec, nullptr) < 0) {
        g_Log(logLevelError, "FFmpeg Plugin :: Failed to open encoder context");
        return errNoCodec;
    }

    pkt = av_packet_alloc();

    swFrame = av_frame_alloc();
    swFrame->format = pixelFormat;
    swFrame->width = static_cast<int>(width);
    swFrame->height = static_cast<int>(height);

    if (av_frame_get_buffer(swFrame, 0) < 0) {
        g_Log(logLevelError, "FFmpeg Plugin :: Failed to allocate frame buffer");
        return errAlloc;
    }

    p_pBuff->SetProperty(pIOPropMagicCookie, propTypeUInt8, ctx->extradata, ctx->extradata_size);
    const uint32_t fourCC = encoderInfo.fourCC == 'avc1' ? 'avcC' : 0;
    p_pBuff->SetProperty(pIOPropMagicCookieType, propTypeUInt32, &fourCC, 1);

    const uint32_t temporal = ctx->has_b_frames;
    p_pBuff->SetProperty(pIOPropTemporalReordering, propTypeUInt32, &temporal, 1);

    return errNone;
}

void FFmpegEncoder::ApplyOptions(AVCodecContext* ctx, UISettingsController& settings) {
    switch (settings.GetQualityMode()) {
        case CQP:
            if (useVaapi) {
                av_opt_set(ctx->priv_data, "rc_mode", "CQP", 0);
                ctx->global_quality = encoderInfo.fourCC == 'av01' ? settings.GetQP() * 4 : settings.GetQP();
            } else {
                av_opt_set_int(ctx->priv_data, encoderInfo.hwAcceleration == Nvenc ? "cq" : "qp", settings.GetQP(), 0);
            }
            break;
        case CRF:
            av_opt_set_int(ctx->priv_data, "crf", settings.GetQP(), 0);
            break;
        case VBR:
            ctx->bit_rate = settings.GetBitRate();
            break;
    }

    if (useVaapi) {
        constexpr int preEncode = 1 << 3;
        constexpr int VBAQ = 1 << 4;
        ctx->compression_level = settings.GetPreset() << 1 | preEncode | VBAQ | 1;
    } else {
        if (const auto preset = encoderInfo.presets.find(settings.GetPreset()); preset != encoderInfo.presets.end()) {
            av_opt_set(ctx->priv_data, "preset", preset->second.c_str(), 0);
        }
    }
}

StatusCode FFmpegEncoder::DoProcess(HostBufferRef* p_pBuff) {
    int ret = 0;

    if (p_pBuff == nullptr || !p_pBuff->IsValid()) {
        ret = avcodec_send_frame(ctx, nullptr);
    } else {
        char* pBuf = nullptr;
        size_t bufSize = 0;

        if (av_frame_make_writable(swFrame) < 0) {
            g_Log(logLevelError, "FFmpeg Plugin :: Failed to make the frame writeable");
            return errFail;
        }

        if (!p_pBuff->LockBuffer(&pBuf, &bufSize)) {
            g_Log(logLevelError, "FFmpeg Plugin :: Failed to lock the buffer");
            return errFail;
        }

        if (pBuf == nullptr || bufSize == 0) {
            g_Log(logLevelError, "FFmpeg Plugin :: No data to encode");
            p_pBuff->UnlockBuffer();
            return errUnsupported;
        }

        uint32_t width, height;
        if (!p_pBuff->GetUINT32(pIOPropWidth, width) || !p_pBuff->GetUINT32(pIOPropHeight, height)) {
            g_Log(logLevelError, "FFmpeg Plugin :: Width/height not set when encoding the frame");
            return errNoParam;
        }

        if (av_image_fill_arrays(swFrame->data, swFrame->linesize, reinterpret_cast<uint8_t*>(pBuf), pixelFormat,
                                 static_cast<int>(width), static_cast<int>(height), 32) < 0) {
            g_Log(logLevelError, "FFmpeg Plugin :: Failed to populate the frame");
            return errFail;
        }

        int64_t pts;
        if (!p_pBuff->GetINT64(pIOPropPTS, pts)) {
            g_Log(logLevelError, "FFmpeg Plugin :: PTS not set when encoding the frame");
            return errNoParam;
        }

        if (useVaapi) {
            AVFrame* hwFrame = av_frame_alloc();
            if (hwFrame == nullptr) return errAlloc;

            if (int err; (err = av_hwframe_get_buffer(ctx->hw_frames_ctx, hwFrame, 0)) < 0) {
                g_Log(logLevelError, "FFmpeg Plugin :: Failed to allocate HW frame. %s", av_err2str(err));
                av_frame_free(&hwFrame);
                return errAlloc;
            }

            if (int err; (err = av_hwframe_transfer_data(hwFrame, swFrame, 0)) < 0) {
                g_Log(logLevelError, "FFmpeg Plugin :: Error while transferring frame data to surface. %s",
                      av_err2str(err));
                av_frame_free(&hwFrame);
                return errUnsupported;
            }

            p_pBuff->UnlockBuffer();

            hwFrame->pts = pts;
            ret = avcodec_send_frame(ctx, hwFrame);

            av_frame_free(&hwFrame);
        } else {
            swFrame->pts = pts;
            ret = avcodec_send_frame(ctx, swFrame);
            p_pBuff->UnlockBuffer();
        }
    }

    if (ret == AVERROR_EOF) {
        av_packet_unref(pkt);
        return errNone;
    }

    if (ret < 0) {
        g_Log(logLevelError, "FFmpeg Plugin :: Failed to encode frame. %s", av_err2str(ret));
        return errFail;
    }

    while (true) {
        ret = avcodec_receive_packet(ctx, pkt);

        if (ret == AVERROR(EAGAIN)) {
            return errMoreData;
        }

        if (ret == AVERROR_EOF) {
            av_packet_unref(pkt);
            return errNone;
        }

        if (ret < 0) {
            g_Log(logLevelError, "FFmpeg Plugin :: Failed to read encoded data. %s", av_err2str(ret));
            av_packet_unref(pkt);
            return errFail;
        }

        HostBufferRef outBuf(false);
        if (!outBuf.IsValid() || !outBuf.Resize(pkt->size)) {
            g_Log(logLevelError, "FFmpeg Plugin :: Failed to resize output buffer");
            av_packet_unref(pkt);
            return errAlloc;
        }

        char* outBufPtr = nullptr;
        size_t outBufSize = 0;

        if (!outBuf.LockBuffer(&outBufPtr, &outBufSize)) {
            g_Log(logLevelError, "FFmpeg Plugin :: Failed to lock the output buffer");
            av_packet_unref(pkt);
            return errAlloc;
        }

        memcpy(outBufPtr, pkt->data, pkt->size);

        outBuf.SetProperty(pIOPropPTS, propTypeInt64, &pkt->pts, 1);
        outBuf.SetProperty(pIOPropDTS, propTypeInt64, &pkt->dts, 1);

        const uint8_t isKeyFrame = pkt->flags & AV_PKT_FLAG_KEY ? 1 : 0;
        outBuf.SetProperty(pIOPropIsKeyFrame, propTypeUInt8, &isKeyFrame, 1);

        av_packet_unref(pkt);

        m_pCallback->SendOutput(&outBuf);
    }
}

void FFmpegEncoder::DoFlush() { DoProcess(nullptr); }

bool FFmpegEncoder::IsEncoderSupported(const EncoderInfo& encoderInfo) {
    bool isEncoderSupported = false;

    const int logLevel = av_log_get_level();
    av_log_set_level(AV_LOG_ERROR);

    AVCodecContext* ctx = nullptr;
    AVBufferRef* hwFramesRef = nullptr;
    AVBufferRef* hwDeviceCtx = nullptr;
    AVHWFramesContext* framesCtx = nullptr;

    const AVCodec* codec = avcodec_find_encoder_by_name(encoderInfo.encoder);
    if (!codec) goto end;

    if (encoderInfo.hwAcceleration == None) {
        isEncoderSupported = true;
        goto end;
    }

    ctx = avcodec_alloc_context3(codec);
    if (!ctx) goto end;

    ctx->pix_fmt = encoderInfo.hwAcceleration == Vaapi ? AV_PIX_FMT_VAAPI : encoderInfo.pixelFormat;
    ctx->time_base = {25, 1};
    ctx->width = 1920;
    ctx->height = 1080;

    if (encoderInfo.hwAcceleration == Vaapi) {
        if (av_hwdevice_ctx_create(&hwDeviceCtx, AV_HWDEVICE_TYPE_VAAPI, nullptr, nullptr, 0) < 0) goto end;
        if (!((hwFramesRef = av_hwframe_ctx_alloc(hwDeviceCtx)))) goto end;

        framesCtx = reinterpret_cast<AVHWFramesContext*>(hwFramesRef->data);
        framesCtx->format = AV_PIX_FMT_VAAPI;
        framesCtx->sw_format = AV_PIX_FMT_NV12;
        framesCtx->width = ctx->width;
        framesCtx->height = ctx->height;
        if (av_hwframe_ctx_init(hwFramesRef) < 0) goto end;

        ctx->hw_frames_ctx = av_buffer_ref(hwFramesRef);
        if (!ctx->hw_frames_ctx) goto end;
    }

    if (avcodec_open2(ctx, codec, nullptr) < 0) goto end;

    isEncoderSupported = true;

end:
    if (ctx != nullptr) avcodec_free_context(&ctx);
    if (hwDeviceCtx != nullptr) av_buffer_unref(&hwDeviceCtx);
    if (hwFramesRef != nullptr) av_buffer_unref(&hwFramesRef);

    av_log_set_level(logLevel);

    return isEncoderSupported;
}

FFmpegEncoder::FFmpegEncoder() = default;

FFmpegEncoder::~FFmpegEncoder() {
    if (ctx != nullptr) avcodec_free_context(&ctx);
    if (hwDeviceCtx != nullptr) av_buffer_unref(&hwDeviceCtx);
    if (pkt != nullptr) av_packet_free(&pkt);
    if (swFrame != nullptr) av_frame_free(&swFrame);
}
