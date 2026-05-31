#include "pixel_format_repacker.h"

#include <cassert>

#if defined(__x86_64__) && !defined(_WIN32) && !defined(__APPLE__) && \
    ((defined(__GNUC__) && !defined(__clang__) && __GNUC__ >= 12) || (defined(__clang__) && __clang_major__ >= 18))
#define TARGET_CLONES __attribute__((target_clones("arch=x86-64-v4", "arch=x86-64-v3", "arch=x86-64-v2", "default")))
#else
#define TARGET_CLONES
#endif

TARGET_CLONES
void convertP010ToYUV420P10(const char* src, AVFrame* dstFrame, const int width, const int height) {
    assert((width & 1) == 0);
    assert((height & 1) == 0);

    auto* __restrict yPlane = reinterpret_cast<uint16_t*>(dstFrame->data[0]);
    auto* __restrict uPlane = reinterpret_cast<uint16_t*>(dstFrame->data[1]);
    auto* __restrict vPlane = reinterpret_cast<uint16_t*>(dstFrame->data[2]);

    const int yStride = dstFrame->linesize[0] / static_cast<int>(sizeof(uint16_t));
    const int uStride = dstFrame->linesize[1] / static_cast<int>(sizeof(uint16_t));
    const int vStride = dstFrame->linesize[2] / static_cast<int>(sizeof(uint16_t));

    const int ySrcStride = width * 2;
    const int uvSrcStride = width * 2;

    const auto* __restrict srcY = reinterpret_cast<const uint16_t*>(src);
    const auto* __restrict srcUV = reinterpret_cast<const uint16_t*>(src + ySrcStride * height);

    // Copy Y plane
    for (int y = 0; y < height; ++y) {
        const auto* __restrict srcRow =
            reinterpret_cast<const uint16_t*>(reinterpret_cast<const uint8_t*>(srcY) + y * ySrcStride);

        uint16_t* __restrict dstRow = yPlane + y * yStride;

        for (int x = 0; x < width; ++x) {
            dstRow[x] = srcRow[x] >> 6;
        }
    }

    // Split UV plane into planar U/V
    const int chromaHeight = height / 2;
    const int chromaWidth = width / 2;

    for (int y = 0; y < chromaHeight; ++y) {
        const auto* __restrict srcRow =
            reinterpret_cast<const uint16_t*>(reinterpret_cast<const uint8_t*>(srcUV) + y * uvSrcStride);

        uint16_t* __restrict dstU = uPlane + y * uStride;
        uint16_t* __restrict dstV = vPlane + y * vStride;

        for (int x = 0; x < chromaWidth; ++x) {
            dstU[x] = srcRow[x * 2 + 0] >> 6;
            dstV[x] = srcRow[x * 2 + 1] >> 6;
        }
    }
}

TARGET_CLONES
void convertUYVY422ToYUV422P(const char* src, AVFrame* dstFrame, const int width, const int height) {
    assert((width & 1) == 0);

    uint8_t* __restrict yPlane = dstFrame->data[0];
    uint8_t* __restrict uPlane = dstFrame->data[1];
    uint8_t* __restrict vPlane = dstFrame->data[2];

    const int yStride = dstFrame->linesize[0];
    const int uStride = dstFrame->linesize[1];
    const int vStride = dstFrame->linesize[2];

    const int srcStride = width * 2;

    for (int y = 0; y < height; ++y) {
        const char* __restrict srcRow = src + y * srcStride;

        uint8_t* __restrict dstY = yPlane + y * yStride;
        uint8_t* __restrict dstU = uPlane + y * uStride;
        uint8_t* __restrict dstV = vPlane + y * vStride;

        for (int x = 0; x < width; x += 2) {
            const uint8_t u = srcRow[0];
            const uint8_t y0 = srcRow[1];
            const uint8_t v = srcRow[2];
            const uint8_t y1 = srcRow[3];

            dstY[0] = y0;
            dstY[1] = y1;

            dstU[0] = u;
            dstV[0] = v;

            srcRow += 4;
            dstY += 2;
            dstU += 1;
            dstV += 1;
        }
    }
}

TARGET_CLONES
void convertV210ToYUV422P10(const char* src, AVFrame* dstFrame, const int width, const int height) {
    assert((width & 1) == 0);

    auto* __restrict yPlane = reinterpret_cast<uint16_t*>(dstFrame->data[0]);
    auto* __restrict uPlane = reinterpret_cast<uint16_t*>(dstFrame->data[1]);
    auto* __restrict vPlane = reinterpret_cast<uint16_t*>(dstFrame->data[2]);

    const int yStride = dstFrame->linesize[0] / static_cast<int>(sizeof(uint16_t));
    const int uStride = dstFrame->linesize[1] / static_cast<int>(sizeof(uint16_t));
    const int vStride = dstFrame->linesize[2] / static_cast<int>(sizeof(uint16_t));

    const int srcStride = (width + 47) / 48 * 128;
    const int fullBlocks = width / 6;
    const int tailPixels = width % 6;

    for (int y = 0; y < height; ++y) {
        const auto* __restrict srcRow = reinterpret_cast<const uint32_t*>(src + y * srcStride);

        uint16_t* __restrict dstY = yPlane + y * yStride;
        uint16_t* __restrict dstU = uPlane + y * uStride;
        uint16_t* __restrict dstV = vPlane + y * vStride;

        for (int b = 0; b < fullBlocks; ++b) {
            const uint32_t w0 = srcRow[0];
            const uint32_t w1 = srcRow[1];
            const uint32_t w2 = srcRow[2];
            const uint32_t w3 = srcRow[3];

            dstU[0] = w0 & 0x3FF;        // U0
            dstY[0] = w0 >> 10 & 0x3FF;  // Y0
            dstV[0] = w0 >> 20 & 0x3FF;  // V0

            dstY[1] = w1 & 0x3FF;        // Y1
            dstU[1] = w1 >> 10 & 0x3FF;  // U1
            dstY[2] = w1 >> 20 & 0x3FF;  // Y2

            dstV[1] = w2 & 0x3FF;        // V1
            dstY[3] = w2 >> 10 & 0x3FF;  // Y3
            dstU[2] = w2 >> 20 & 0x3FF;  // U2

            dstY[4] = w3 & 0x3FF;        // Y4
            dstV[2] = w3 >> 10 & 0x3FF;  // V2
            dstY[5] = w3 >> 20 & 0x3FF;  // Y5

            srcRow += 4;
            dstY += 6;
            dstU += 3;
            dstV += 3;
        }

        // Handle trailing pixels
        if (tailPixels > 0) {
            const uint32_t w0 = srcRow[0];
            const uint32_t w1 = srcRow[1];
            const uint32_t w2 = srcRow[2];

            dstY[0] = w0 >> 10 & 0x3FF;  // Y0
            dstY[1] = w1 & 0x3FF;        // Y1
            dstU[0] = w0 & 0x3FF;        // U0
            dstV[0] = w0 >> 20 & 0x3FF;  // V0

            if (tailPixels > 2) {
                dstY[2] = w1 >> 20 & 0x3FF;  // Y2
                dstY[3] = w2 >> 10 & 0x3FF;  // Y3
                dstU[1] = w1 >> 10 & 0x3FF;  // U1
                dstV[1] = w2 & 0x3FF;        // V1
            }
        }
    }
}

TARGET_CLONES
void convertYUV444P10MSBToYUV444P10(const char* src, AVFrame* dstFrame, const int width, const int height) {
    const int srcStride = width * 2;
    const int planeSize = srcStride * height;

    for (int plane = 0; plane < 3; ++plane) {
        auto* __restrict dstPlane = reinterpret_cast<uint16_t*>(dstFrame->data[plane]);
        const auto* __restrict srcPlane = reinterpret_cast<const uint16_t*>(src + plane * planeSize);

        const int dstStride = dstFrame->linesize[plane] / 2;

        for (int y = 0; y < height; ++y) {
            const auto* __restrict srcRow =
                reinterpret_cast<const uint16_t*>(reinterpret_cast<const uint8_t*>(srcPlane) + y * srcStride);

            uint16_t* __restrict dstRow = dstPlane + y * dstStride;

            for (int x = 0; x < width; ++x) {
                dstRow[x] = srcRow[x] >> 6;
            }
        }
    }
}
