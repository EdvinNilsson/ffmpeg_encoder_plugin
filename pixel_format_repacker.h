#pragma once

extern "C" {
#include <libavutil/frame.h>
}

typedef void(PixelFormatRepacker)(const char* src, AVFrame* dstFrame, int width, int height);

void convertP010ToYUV420P10(const char* src, AVFrame* dstFrame, int width, int height);
void convertUYVY422ToYUV422P(const char* src, AVFrame* dstFrame, int width, int height);
void convertV210ToYUV422P10(const char* src, AVFrame* dstFrame, int width, int height);
void convertYUV444P10MSBToYUV444P10(const char* src, AVFrame* dstFrame, int width, int height);
