#include "ffmpeg_encoder.h"

extern "C" {
#include <libavutil/log.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
}

#undef av_err2str
av_always_inline std::string av_err2string(int errnum) {
    char str[AV_ERROR_MAX_STRING_SIZE];
    return av_make_error_string(str, AV_ERROR_MAX_STRING_SIZE, errnum);
}
#define av_err2str(err) av_err2string(err).c_str()

StatusCode FFmpegEncoder::DoInit(HostPropertyCollectionRef* p_pProps) { return errNone; }

StatusCode FFmpegEncoder::RegisterCodecs(HostListRef* list, const EncoderInfo& encoderInfo) {
    for (int i = 0; i < static_cast<int>(encoderInfo.formats.size()); ++i) {
        const EncoderFormat& format = encoderInfo.formats[i];

        HostPropertyCollectionRef codecInfo;
        if (!codecInfo.IsValid()) {
            return errAlloc;
        }

        if (!IsEncoderSupported(encoderInfo, i)) {
            g_Log(logLevelWarn, "FFmpeg Plugin :: Encoder '%s' is not supported with format '%s'", encoderInfo.encoder,
                  av_get_pix_fmt_name(format.pixelFormat));
            continue;
        }

        uint8_t uuid[16];
        memcpy(uuid, encoderInfo.UUID, sizeof(uuid));
        uuid[15] += i;
        codecInfo.SetProperty(pIOPropUUID, propTypeUInt8, uuid, 16);

        const char* codecGroup = encoderInfo.codecGroup;
        codecInfo.SetProperty(pIOPropGroup, propTypeString, codecGroup, static_cast<int>(strlen(codecGroup)));

        const char* codecName = format.codecName;
        codecInfo.SetProperty(pIOPropName, propTypeString, codecName, static_cast<int>(strlen(codecName)));

        codecInfo.SetProperty(pIOPropFourCC, propTypeUInt32, &encoderInfo.fourCC, 1);

        constexpr uint32_t vMediaVideo = mediaVideo;
        codecInfo.SetProperty(pIOPropMediaType, propTypeUInt32, &vMediaVideo, 1);

        constexpr uint32_t vDirection = dirEncode;
        codecInfo.SetProperty(pIOPropCodecDirection, propTypeUInt32, &vDirection, 1);

        codecInfo.SetProperty(pIOPropColorModel, propTypeUInt32, &format.colorModel, 1);
        codecInfo.SetProperty(pIOPropHSubsampling, propTypeUInt8, &format.hSubsampling, 1);
        codecInfo.SetProperty(pIOPropVSubsampling, propTypeUInt8, &format.vSubsampling, 1);

        constexpr uint8_t dataRange[] = {0, 1};
        codecInfo.SetProperty(pIOPropDataRange, propTypeUInt8, &dataRange, sizeof(dataRange));

        codecInfo.SetProperty(pIOPropBitDepth, propTypeUInt32, &format.bitDepth, 1);
        codecInfo.SetProperty(pIOPropBitsPerSample, propTypeUInt32, &format.bitDepth, 1);

        constexpr uint32_t temp = 0;
        codecInfo.SetProperty(pIOPropTemporalReordering, propTypeUInt32, &temp, 1);

        constexpr uint8_t fieldSupport = fieldProgressive | fieldTop | fieldBottom;
        codecInfo.SetProperty(pIOPropFieldOrder, propTypeUInt8, &fieldSupport, 1);

        constexpr uint8_t threadSafe = 1;
        codecInfo.SetProperty(pIOPropThreadSafe, propTypeUInt8, &threadSafe, 1);

        const bool hwAcc = encoderInfo.hwAcceleration != None;
        codecInfo.SetProperty(pIOPropHWAcc, propTypeUInt8, &hwAcc, 1);

        const std::vector<std::string> containerVec = {"mov", "mp4", "mkv"};
        std::string valStrings;
        for (size_t j = 0; j < containerVec.size(); ++j) {
            valStrings.append(containerVec[j]);
            if (j < containerVec.size() - 1) {
                valStrings.append(1, '\0');
            }
        }

        codecInfo.SetProperty(pIOPropContainerList, propTypeString, valStrings.c_str(),
                              static_cast<int>(valStrings.size()));

        if (!list->Append(&codecInfo)) {
            return errFail;
        }
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

    int16_t colorMatrix{};
    int16_t colorPrimaries{};
    int16_t transferFunction{};
    uint8_t dataRange{};

    if (!p_pBuff->GetINT16(pIOColorMatrix, colorMatrix)) return errNoParam;
    if (!p_pBuff->GetINT16(pIOPropColorPrimaries, colorPrimaries)) return errNoParam;
    if (!p_pBuff->GetINT16(pIOTransferCharacteristics, transferFunction)) return errNoParam;
    if (!p_pBuff->GetUINT8(pIOPropDataRange, dataRange)) return errNoParam;

    constexpr uint8_t isMultiPass = 0;
    p_pBuff->SetProperty(pIOPropMultiPass, propTypeUInt8, &isMultiPass, 1);

    const EncoderFormat& format = encoderInfo.formats[formatIndex];

    width = static_cast<int>(commonProps.GetWidth());
    height = static_cast<int>(commonProps.GetHeight());
    frameRateNum = commonProps.GetFrameRateNum();
    frameRateDen = commonProps.GetFrameRateDen();
    pixelFormat = format.pixelFormat;
    srcPixelFormat = format.srcPixelFormat;
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
    ctx->width = width;
    ctx->height = height;
    ctx->time_base = {static_cast<int>(frameRateDen), static_cast<int>(frameRateNum)};
    ctx->framerate = {static_cast<int>(frameRateNum), static_cast<int>(frameRateDen)};
    ctx->thread_count = 0;
    ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    ctx->colorspace = static_cast<AVColorSpace>(colorMatrix);
    ctx->color_primaries = static_cast<AVColorPrimaries>(colorPrimaries);
    ctx->color_trc = static_cast<AVColorTransferCharacteristic>(transferFunction);
    ctx->color_range = dataRange == 1 ? AVCOL_RANGE_JPEG : AVCOL_RANGE_MPEG;

    if (const StatusCode err = ApplyOptions(ctx, *settings, p_pBuff); err != errNone) return err;

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
        framesCtx->width = width;
        framesCtx->height = height;
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

    thread_local std::string logs;
    logs.clear();

    auto logCallback = [](void* ptr, int level, const char* fmt, va_list vl) {
        if (level <= AV_LOG_WARNING) {
            if (!strcmp(fmt, "Invalid value for %s: %s.\n") || !strcmp(fmt, "Error parsing option '%s = %s'.\n") ||
                !strcmp(fmt, "Error parsing option %s: %s.\n") || !strcmp(fmt, "Unknown option: %s.\n") ||
                level <= AV_LOG_ERROR) {
                va_list vl_copy;
                va_copy(vl_copy, vl);

                char line[1024];
                vsnprintf(line, sizeof(line), fmt, vl_copy);
                logs += line;

                va_end(vl_copy);
            }
        }
        av_log_default_callback(ptr, level, fmt, vl);
    };

    av_log_set_callback(logCallback);

    const int ret = avcodec_open2(ctx, codec, nullptr);

    av_log_set_callback(av_log_default_callback);

    if (ret < 0) {
        g_Log(logLevelError, "FFmpeg Plugin :: Failed to open encoder context");
        if (!logs.empty()) {
            p_pBuff->SetProperty(pIOCustomErrorString, propTypeString, logs.c_str(), static_cast<int>(logs.size()));
        }
        return errNoCodec;
    }

    if (!logs.empty()) {
        g_Log(logLevelError, "FFmpeg Plugin :: Invalid custom encoder params");
        p_pBuff->SetProperty(pIOCustomErrorString, propTypeString, logs.c_str(), static_cast<int>(logs.size()));
        return errInvalidParam;
    }

    pkt = av_packet_alloc();

    swFrame = av_frame_alloc();
    swFrame->format = pixelFormat;
    swFrame->width = width;
    swFrame->height = height;

    av_image_fill_linesizes(swFrame->linesize, pixelFormat, width);

    if (srcPixelFormat != AV_PIX_FMT_NONE) {
        swsCtx =
            sws_getContext(width, height, srcPixelFormat, width, height, pixelFormat, 0, nullptr, nullptr, nullptr);
    }

    p_pBuff->SetProperty(pIOPropMagicCookie, propTypeUInt8, ctx->extradata, ctx->extradata_size);
    const uint32_t fourCC = encoderInfo.fourCC == 'avc1' ? 'avcC' : 0;
    p_pBuff->SetProperty(pIOPropMagicCookieType, propTypeUInt32, &fourCC, 1);

    const uint32_t temporal = ctx->has_b_frames;
    p_pBuff->SetProperty(pIOPropTemporalReordering, propTypeUInt32, &temporal, 1);

    return errNone;
}

StatusCode FFmpegEncoder::ApplyOptions(AVCodecContext* ctx, UISettingsController& settings, HostBufferRef* p_pBuff) {
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

    if (encoderInfo.customParamsKey != nullptr && !settings.GetCustomParams().empty()) {
        if (av_opt_set(ctx->priv_data, encoderInfo.customParamsKey, settings.GetCustomParams().c_str(), 0) < 0) {
            const std::string msg =
                "Invalid format for encoder params (" + std::string(encoderInfo.customParamsKey) + ").";
            g_Log(logLevelError, "FFmpeg Plugin :: %s", msg.c_str());
            p_pBuff->SetProperty(pIOCustomErrorString, propTypeString, msg.c_str(), static_cast<int>(msg.size()));
            return errInvalidParam;
        }
    }

    return errNone;
}

StatusCode FFmpegEncoder::DoProcess(HostBufferRef* p_pBuff) {
    int ret = 0;

    if (p_pBuff == nullptr || !p_pBuff->IsValid()) {
        ret = avcodec_send_frame(ctx, nullptr);
    } else {
        char* pBuf = nullptr;
        size_t bufSize = 0;

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

        if (srcPixelFormat != AV_PIX_FMT_NONE) {
            AVFrame* src = av_frame_alloc();
            if (src == nullptr) return errAlloc;

            src->format = srcPixelFormat;
            src->width = static_cast<int>(width);
            src->height = static_cast<int>(height);

            if (av_image_fill_linesizes(src->linesize, srcPixelFormat, src->width) < 0) {
                g_Log(logLevelError, "FFmpeg Plugin :: Failed to fill linesizes");
                av_frame_free(&src);
                return errFail;
            }

            if (av_image_fill_pointers(src->data, srcPixelFormat, src->height, reinterpret_cast<uint8_t*>(pBuf),
                                       src->linesize) < 0) {
                g_Log(logLevelError, "FFmpeg Plugin :: Failed to populate the frame");
                av_frame_free(&src);
                return errFail;
            }

            av_frame_unref(swFrame);
            swFrame->format = pixelFormat;
            swFrame->width = src->width;
            swFrame->height = src->height;

            if (av_frame_get_buffer(swFrame, 32) < 0) {
                g_Log(logLevelError, "FFmpeg Plugin :: Failed to access the frame buffer");
                av_frame_free(&src);
                return errFail;
            }

            sws_scale(swsCtx, src->data, src->linesize, 0, src->height, swFrame->data, swFrame->linesize);

            av_frame_free(&src);
        } else {
            if (av_image_fill_pointers(swFrame->data, pixelFormat, static_cast<int>(height),
                                       reinterpret_cast<uint8_t*>(pBuf), swFrame->linesize) < 0) {
                g_Log(logLevelError, "FFmpeg Plugin :: Failed to populate the frame");
                return errFail;
            }
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

bool FFmpegEncoder::IsEncoderSupported(const EncoderInfo& encoderInfo, const int formatIndex) {
    bool isEncoderSupported = false;

    const int logLevel = av_log_get_level();
    av_log_set_level(AV_LOG_ERROR);

    AVCodecContext* ctx = nullptr;
    AVBufferRef* hwFramesRef = nullptr;
    AVBufferRef* hwDeviceCtx = nullptr;
    AVHWFramesContext* framesCtx = nullptr;

    const AVPixelFormat pixelFormat = encoderInfo.formats[formatIndex].pixelFormat;

    const AVCodec* codec = avcodec_find_encoder_by_name(encoderInfo.encoder);
    if (!codec) goto end;

    if (encoderInfo.hwAcceleration == None) {
        const void* configs = nullptr;
        int numConfigs;
        avcodec_get_supported_config(nullptr, codec, AV_CODEC_CONFIG_PIX_FORMAT, 0, &configs, &numConfigs);
        if (configs) {
            const auto* pixFmts = static_cast<const AVPixelFormat*>(configs);
            for (int i = 0; i < numConfigs; ++i) {
                if (pixFmts[i] == pixelFormat) {
                    isEncoderSupported = true;
                    break;
                }
            }
        } else {
            isEncoderSupported = true;
        }
        goto end;
    }

    ctx = avcodec_alloc_context3(codec);
    if (!ctx) goto end;

    ctx->pix_fmt = encoderInfo.hwAcceleration == Vaapi ? AV_PIX_FMT_VAAPI : pixelFormat;
    ctx->time_base = {25, 1};
    ctx->width = 1920;
    ctx->height = 1080;

    if (encoderInfo.hwAcceleration == Vaapi) {
        if (av_hwdevice_ctx_create(&hwDeviceCtx, AV_HWDEVICE_TYPE_VAAPI, nullptr, nullptr, 0) < 0) goto end;
        if (!((hwFramesRef = av_hwframe_ctx_alloc(hwDeviceCtx)))) goto end;

        framesCtx = reinterpret_cast<AVHWFramesContext*>(hwFramesRef->data);
        framesCtx->format = AV_PIX_FMT_VAAPI;
        framesCtx->sw_format = pixelFormat;
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
    if (swsCtx != nullptr) sws_freeContext(swsCtx);
    if (pkt != nullptr) av_packet_free(&pkt);
    if (swFrame != nullptr) av_frame_free(&swFrame);
}
