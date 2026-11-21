#include "audio_encoder.h"

#include <algorithm>
#include <cstring>
#include <utility>

#include "ffmpeg_utils.h"

extern "C" {
#include <libavcodec/codec.h>
#include <libavutil/channel_layout.h>
#include <libavutil/opt.h>
#include <libavutil/samplefmt.h>
}

StatusCode AudioEncoder::DoInit(HostPropertyCollectionRef* p_pProps) { return errNone; }

StatusCode AudioEncoder::RegisterCodecs(HostListRef* list, const AudioEncoderInfo& encoderInfo) {
    for (int i = 0; i < static_cast<int>(encoderInfo.formats.size()); ++i) {
        const AudioEncoderFormat& format = encoderInfo.formats[i];

        HostPropertyCollectionRef codecInfo;
        if (!codecInfo.IsValid()) {
            return errAlloc;
        }

        if (!IsEncoderSupported(encoderInfo)) {
            g_Log(logLevelWarn, "FFmpeg Plugin :: Encoder '%s' is not supported", encoderInfo.encoder);
            continue;
        }

        uint8_t uuid[16];
        std::memcpy(uuid, encoderInfo.UUID, sizeof(uuid));
        uuid[15] += i;
        codecInfo.SetProperty(pIOPropUUID, propTypeUInt8, uuid, 16);

        codecInfo.SetProperty(pIOPropName, propTypeString, format.codecName,
                              static_cast<int>(strlen(format.codecName)));

        codecInfo.SetProperty(pIOPropFourCC, propTypeUInt32, &encoderInfo.fourCC, 1);

        constexpr uint32_t vMediaAudio = mediaAudio;
        codecInfo.SetProperty(pIOPropMediaType, propTypeUInt32, &vMediaAudio, 1);

        constexpr uint32_t vDirection = dirEncode;
        codecInfo.SetProperty(pIOPropCodecDirection, propTypeUInt32, &vDirection, 1);

        codecInfo.SetProperty(pIOPropBitDepth, propTypeUInt32, &format.bitDepth, 1);
        codecInfo.SetProperty(pIOPropBitsPerSample, propTypeUInt32, &format.bitDepth, 1);

        constexpr uint8_t vIsFloat = 0;
        codecInfo.SetProperty(pIOPropIsFloat, propTypeUInt8, &vIsFloat, 1);

        const std::vector<uint32_t> samplingRates({44100, 48000, 96000});
        codecInfo.SetProperty(pIOPropSamplingRate, propTypeUInt32, samplingRates.data(),
                              static_cast<int>(samplingRates.size()));

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

StatusCode AudioEncoder::GetEncoderSettings(HostPropertyCollectionRef* values, HostListRef* settingsList,
                                            const AudioEncoderInfo& encoderInfo) {
    UISettingsController settings(encoderInfo);
    settings.Load(values);

    return settings.Render(settingsList);
}

StatusCode AudioEncoder::DoOpen(HostBufferRef* p_pBuff) {
    settings = std::make_unique<UISettingsController>(encoderInfo);
    settings->Load(p_pBuff);

    const uint64_t bitRate = settings->GetBitRate();
    if (bitRate > 0) {
        p_pBuff->SetProperty(pIOPropBitRate, propTypeUInt64, &bitRate, 1);
    }

    if (!p_pBuff->GetUINT32(pIOPropBitDepth, bitDepth)) return errNoParam;

    uint32_t samplingRate = 0;
    if (!p_pBuff->GetUINT32(pIOPropSamplingRate, samplingRate)) return errNoParam;

    uint32_t numChannels = 0;
    if (!p_pBuff->GetUINT32(pIOPropNumChannels, numChannels)) return errNoParam;

    if (samplingRate == 0 || numChannels == 0 || bitDepth == 0) {
        g_Log(logLevelError, "FFmpeg Plugin :: Invalid sampling rate, channel count or bit depth");
        return errInvalidParam;
    }

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

    ctx->bit_rate = static_cast<int64_t>(bitRate);
    ctx->sample_fmt = AV_SAMPLE_FMT_FLTP;
    ctx->sample_rate = static_cast<int>(samplingRate);
    ctx->time_base = {.num = 1, .den = static_cast<int>(samplingRate)};
    ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    av_channel_layout_default(&ctx->ch_layout, static_cast<int>(numChannels));

    int ret = avcodec_open2(ctx, codec, nullptr);
    if (ret < 0) {
        g_Log(logLevelError, "FFmpeg Plugin :: Failed to open encoder context");
        return errNoCodec;
    }

    pkt = av_packet_alloc();
    if (pkt == nullptr) return errAlloc;

    frame = av_frame_alloc();
    if (frame == nullptr) return errAlloc;
    frame->nb_samples = ctx->frame_size;
    frame->format = ctx->sample_fmt;
    frame->sample_rate = ctx->sample_rate;

    ret = av_channel_layout_copy(&frame->ch_layout, &ctx->ch_layout);
    if (ret < 0) return errFail;

    frameSampleIndex = 0;
    numberOfChannels = static_cast<int>(numChannels);
    bytesPerSample = static_cast<int>(bitDepth) / 8;
    pts = 0;

    if (av_frame_get_buffer(frame, 0) < 0) {
        g_Log(logLevelError, "FFmpeg Plugin :: Failed to allocate frame");
        return errAlloc;
    }

    p_pBuff->SetProperty(pIOPropMagicCookie, propTypeUInt8, ctx->extradata, ctx->extradata_size);
    constexpr uint32_t fourCC = 'esds';
    p_pBuff->SetProperty(pIOPropMagicCookieType, propTypeUInt32, &fourCC, 1);

    uint32_t profile;
    if (AV_PROFILE_AAC_MAIN <= ctx->profile && ctx->profile <= AV_PROFILE_AAC_LTP) {
        profile = ctx->profile + 1;
    } else {
        const char* msg = "Unsupported AAC profile.";
        g_Log(logLevelWarn, "FFmpeg Plugin :: %s", msg);
        p_pBuff->SetProperty(pIOCustomErrorString, propTypeString, msg, static_cast<int>(strlen(msg)));
        return errUnsupported;
    }

    constexpr uint32_t sampleRates[] = {96000, 88200, 64000, 48000, 44100, 32000, 24000,
                                        22050, 16000, 12000, 11025, 8000,  7350};
    const auto it = std::find(std::begin(sampleRates), std::end(sampleRates), ctx->sample_rate);
    if (it == std::end(sampleRates)) {
        const char* msg = "Unsupported sample rate.";
        g_Log(logLevelWarn, "FFmpeg Plugin :: %s", msg);
        p_pBuff->SetProperty(pIOCustomErrorString, propTypeString, msg, static_cast<int>(strlen(msg)));
        return errUnsupported;
    }
    const uint32_t sampleRateIndex = it - std::begin(sampleRates);

    uint32_t channels = ctx->ch_layout.nb_channels;
    if (channels == 8) {
        channels = 7;
    } else if (channels < 1 || channels == 7 || channels > 8) {
        const char* msg = "Unsupported number of audio channels.";
        g_Log(logLevelWarn, "FFmpeg Plugin :: %s", msg);
        p_pBuff->SetProperty(pIOCustomErrorString, propTypeString, msg, static_cast<int>(strlen(msg)));
        return errUnsupported;
    }

    adtsHeader[0] = 0xFF;
    adtsHeader[1] = 0xF1;
    adtsHeader[2] = static_cast<uint8_t>((profile << 6) | (sampleRateIndex << 2) | ((channels >> 2) & 1));
    adtsHeader[3] = static_cast<uint8_t>((channels & 3) << 6);
    adtsHeader[4] = 0;
    adtsHeader[5] = 0x1F;
    adtsHeader[6] = 0xFC;

    return errNone;
}

StatusCode AudioEncoder::DoProcess(HostBufferRef* p_pBuff) {
    int ret;

    int samples = 0;
    int bufferSampleIndex = 0;

    char* pBuf = nullptr;
    size_t bufSize = 0;

    if (p_pBuff == nullptr || !p_pBuff->IsValid()) {
        if (frameSampleIndex > 0) {
            frame->pts = pts;
            frame->nb_samples = frameSampleIndex;
            pts += frame->nb_samples;
            frameSampleIndex = 0;
            ret = avcodec_send_frame(ctx, frame);
        } else {
            ret = avcodec_send_frame(ctx, nullptr);
        }

        goto receive_packets;
    }

    if (p_pBuff->LockBuffer(&pBuf, &bufSize)) {
        samples = static_cast<int>(bufSize) / (numberOfChannels * bytesPerSample);

        for (bufferSampleIndex = 0; bufferSampleIndex < samples;) {
            ret = av_frame_make_writable(frame);
            if (ret < 0) {
                g_Log(logLevelError, "FFmpeg Plugin :: Failed to make the frame writable. %s",
                      av_err2string(ret).c_str());
                p_pBuff->UnlockBuffer();
                return errFail;
            }

            for (; bufferSampleIndex < samples && frameSampleIndex < frame->nb_samples;
                 bufferSampleIndex++, frameSampleIndex++) {
                const auto src = reinterpret_cast<uint8_t*>(pBuf);
                for (int ch = 0; ch < numberOfChannels; ch++) {
                    const int idx = (bufferSampleIndex * numberOfChannels + ch) * bytesPerSample;
                    const auto dst = reinterpret_cast<float*>(frame->data[ch]);
                    if (bitDepth == 24) {
                        int32_t sample = 0;
                        std::memcpy(reinterpret_cast<uint8_t*>(&sample) + 1, &src[idx], 3);
                        sample >>= 8;
                        dst[frameSampleIndex] = static_cast<float>(sample) / 8388608.0f;
                    } else {
                        int16_t sample;
                        std::memcpy(&sample, &src[idx], sizeof(sample));
                        dst[frameSampleIndex] = static_cast<float>(sample) / 32768.0f;
                    }
                }
            }

            if (bufferSampleIndex == samples) {
                p_pBuff->UnlockBuffer();
                return errNone;
            }

            frameSampleIndex = 0;

            frame->pts = pts;
            pts += frame->nb_samples;

            ret = avcodec_send_frame(ctx, frame);

        receive_packets:

            if (ret < 0 && ret != AVERROR_EOF) {
                g_Log(logLevelError, "FFmpeg Plugin :: Failed to encode frame. %s", av_err2string(ret).c_str());
                if (p_pBuff != nullptr) p_pBuff->UnlockBuffer();
                return errFail;
            }

            while (true) {
                ret = avcodec_receive_packet(ctx, pkt);

                if (ret == AVERROR(EAGAIN)) break;

                if (ret == AVERROR_EOF) {
                    av_packet_unref(pkt);
                    if (p_pBuff != nullptr) p_pBuff->UnlockBuffer();
                    return errNone;
                }

                if (ret < 0) {
                    g_Log(logLevelError, "FFmpeg Plugin :: Failed to read encoded data. %s",
                          av_err2string(ret).c_str());
                    av_packet_unref(pkt);
                    if (p_pBuff != nullptr) p_pBuff->UnlockBuffer();
                    return errFail;
                }

                HostBufferRef outBuf(false);
                if (!outBuf.IsValid() || !outBuf.Resize(sizeof(adtsHeader) + pkt->size)) {
                    g_Log(logLevelError, "FFmpeg Plugin :: Failed to resize output buffer");
                    av_packet_unref(pkt);
                    if (p_pBuff != nullptr) p_pBuff->UnlockBuffer();
                    return errAlloc;
                }

                char* outBufPtr = nullptr;
                size_t outBufSize = 0;

                if (!outBuf.LockBuffer(&outBufPtr, &outBufSize)) {
                    g_Log(logLevelError, "FFmpeg Plugin :: Failed to lock the output buffer");
                    av_packet_unref(pkt);
                    if (p_pBuff != nullptr) p_pBuff->UnlockBuffer();
                    return errAlloc;
                }

                std::memcpy(outBufPtr, adtsHeader, sizeof(adtsHeader));
                outBufPtr[3] = static_cast<char>((adtsHeader[3] & 0xFC) | ((outBufSize >> 11) & 0x03));
                outBufPtr[4] = static_cast<char>((outBufSize >> 3) & 0xFF);
                outBufPtr[5] = static_cast<char>(((outBufSize & 0x07) << 5) | 0x1F);

                std::memcpy(outBufPtr + sizeof(adtsHeader), pkt->data, pkt->size);

                outBuf.SetProperty(pIOPropPTS, propTypeInt64, &pkt->pts, 1);
                outBuf.SetProperty(pIOPropDTS, propTypeInt64, &pkt->dts, 1);
                outBuf.SetProperty(pIOPropDuration, propTypeInt64, &pkt->duration, 1);

                av_packet_unref(pkt);

                outBuf.UnlockBuffer();
                m_pCallback->SendOutput(&outBuf);
            }
        }
        if (p_pBuff != nullptr) p_pBuff->UnlockBuffer();
    }
    return errNone;
}

void AudioEncoder::DoFlush() { DoProcess(nullptr); }

bool AudioEncoder::IsEncoderSupported(const AudioEncoderInfo& encoderInfo) {
    return avcodec_find_encoder_by_name(encoderInfo.encoder) != nullptr;
}

AudioEncoder::AudioEncoder(AudioEncoderInfo encoderInfo, const int formatIndex)
    : encoderInfo(std::move(encoderInfo)), formatIndex(formatIndex) {}

AudioEncoder::~AudioEncoder() {
    if (ctx != nullptr) avcodec_free_context(&ctx);
    if (frame != nullptr) av_frame_free(&frame);
    if (pkt != nullptr) av_packet_free(&pkt);
}
