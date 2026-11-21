#pragma once

#include <string>

extern "C" {
#include <libavutil/error.h>
}

inline std::string av_err2string(const int err) {
    char str[AV_ERROR_MAX_STRING_SIZE];
    av_make_error_string(str, AV_ERROR_MAX_STRING_SIZE, err);
    return std::string(str);
}
