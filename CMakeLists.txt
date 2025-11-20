cmake_minimum_required(VERSION 3.20)
project(ffmpeg_encoder_plugin)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(BASE_DIR "${CMAKE_SOURCE_DIR}")
set(FFMPEG_DIR "${BASE_DIR}/ffmpeg")
set(WRAPPER_DIR "${BASE_DIR}/wrapper")

if (NOT MSVC)
    set(CMAKE_CXX_FLAGS "-fPIC -Wall -Wno-unused-variable -Wno-multichar")
endif ()

find_path(AVCODEC_INCLUDE_DIR libavcodec/avcodec.h HINTS "${FFMPEG_DIR}/include")
find_library(AVCODEC_LIBRARY avcodec HINTS "${FFMPEG_DIR}/lib")

find_path(AVUTIL_INCLUDE_DIR libavutil/avutil.h HINTS "${FFMPEG_DIR}/include")
find_library(AVUTIL_LIBRARY avutil HINTS "${FFMPEG_DIR}/lib")

include_directories(include wrapper ${AVCODEC_INCLUDE_DIR} ${AVUTIL_INCLUDE_DIR})

set(SRCS
        av1_encoder.cpp
        av1_nvenc_encoder.cpp
        ffmpeg_encoder.cpp
        h264_encoder.cpp
        h264_nvenc_encoder.cpp
        h265_encoder.cpp
        h265_nvenc_encoder.cpp
        plugin.cpp
        svt_av1_encoder.cpp
        uisettings_controller.cpp
        x264_encoder.cpp
        x265_encoder.cpp
)

set(HEADERS
        av1_encoder.h
        av1_nvenc_encoder.h
        ffmpeg_encoder.h
        h264_encoder.h
        h264_nvenc_encoder.h
        h265_encoder.h
        h265_nvenc_encoder.h
        plugin.h
        svt_av1_encoder.h
        uisettings_controller.h
        x264_encoder.h
        x265_encoder.h
)

add_subdirectory(${WRAPPER_DIR})

add_library(${PROJECT_NAME} SHARED ${SRCS} ${HEADERS})

set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")
set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".dvcp")

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_options(${PROJECT_NAME} PRIVATE -shared -static-libstdc++ -static-libgcc)
else ()
    target_link_options(${PROJECT_NAME} PRIVATE -dynamiclib)
endif ()

target_link_libraries(${PROJECT_NAME} PRIVATE host_plugin ${AVCODEC_LIBRARY} ${AVUTIL_LIBRARY})
