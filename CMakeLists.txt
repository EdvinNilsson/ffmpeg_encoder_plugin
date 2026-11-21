cmake_minimum_required(VERSION 3.20)
project(ffmpeg_encoder_plugin)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(FFMPEG_DIR "${BASE_DIR}/ffmpeg")
set(WRAPPER_DIR "${BASE_DIR}/wrapper")

find_path(FFMPEG_INCLUDE_DIR libavcodec/avcodec.h HINTS "${FFMPEG_DIR}/include")
find_path(VULKAN_INCLUDE_DIR vulkan/vulkan.h HINTS "${FFMPEG_DIR}/include")
find_library(AVCODEC_LIBRARY avcodec HINTS "${FFMPEG_DIR}/lib")
find_library(AVUTIL_LIBRARY avutil HINTS "${FFMPEG_DIR}/lib")
find_library(VA_LIBRARY va)

get_filename_component(AVCODEC_EXT "${AVCODEC_LIBRARY}" EXT)
if (AVCODEC_EXT STREQUAL ".a")
    set(FFMPEG_IS_STATIC TRUE)
endif ()

if (FFMPEG_IS_STATIC)
    find_package(PkgConfig REQUIRED)

    set(PKG_CONFIG_USE_STATIC_LIBS ON)

    set(ENV{PKG_CONFIG_PATH}
            "${FFMPEG_DIR}/lib/pkgconfig:$ENV{PKG_CONFIG_PATH}")

    pkg_check_modules(AVCODEC REQUIRED IMPORTED_TARGET libavcodec)

    find_library(X264_LIBRARY x264 HINTS "${FFMPEG_DIR}/lib" REQUIRED)
    find_library(X265_LIBRARY x265 HINTS "${FFMPEG_DIR}/lib" REQUIRED)
    find_library(SVTAV1_LIBRARY SvtAv1Enc HINTS "${FFMPEG_DIR}/lib" REQUIRED)

    set(FFMPEG_SYSTEM_LIBS ${AVCODEC_STATIC_LIBRARIES})

    list(REMOVE_ITEM FFMPEG_SYSTEM_LIBS
            avcodec
            avutil
            x264
            x265
            SvtAv1Enc
            stdc++
            gcc_s
            gcc
    )
endif ()

include_directories(include wrapper ${FFMPEG_INCLUDE_DIR})

set(SRCS
        av1_nvenc_encoder.cpp
        av1_vaapi_encoder.cpp
        av1_vulkan_encoder.cpp
        ffmpeg_encoder.cpp
        h264_nvenc_encoder.cpp
        h264_vaapi_encoder.cpp
        h264_vulkan_encoder.cpp
        h265_nvenc_encoder.cpp
        h265_vaapi_encoder.cpp
        h265_vulkan_encoder.cpp
        hw_encoder_capabilities.cpp
        pixel_format_repacker.cpp
        plugin.cpp
        svt_av1_encoder.cpp
        uisettings_controller.cpp
        x264_encoder.cpp
        x265_encoder.cpp
)

set(HEADERS
        av1_nvenc_encoder.h
        av1_vaapi_encoder.h
        av1_vulkan_encoder.h
        ffmpeg_encoder.h
        h264_nvenc_encoder.h
        h264_vaapi_encoder.h
        h264_vulkan_encoder.h
        h265_nvenc_encoder.h
        h265_vaapi_encoder.h
        h265_vulkan_encoder.h
        hw_encoder_capabilities.h
        pixel_format_repacker.h
        plugin.h
        svt_av1_encoder.h
        uisettings_controller.h
        x264_encoder.h
        x265_encoder.h
)

add_subdirectory(${WRAPPER_DIR})

add_library(${PROJECT_NAME} SHARED ${SRCS} ${HEADERS})

if (VULKAN_INCLUDE_DIR)
    target_include_directories(${PROJECT_NAME} PRIVATE ${VULKAN_INCLUDE_DIR})
endif ()

target_compile_definitions(${PROJECT_NAME} PRIVATE __STDC_CONSTANT_MACROS)

if (NOT MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE -fPIC -Wall -Wno-unused-variable -Wno-multichar)
endif ()

set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")
set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".dvcp")

if (CMAKE_SYSTEM_NAME STREQUAL "Linux" OR MINGW)
    target_link_options(${PROJECT_NAME} PRIVATE -Wl,--no-undefined -Wl,-Bsymbolic)

    if (FFMPEG_IS_STATIC)
        target_link_options(${PROJECT_NAME} PRIVATE
                -static-libstdc++
                -static-libgcc
        )
    endif ()
else ()
    target_link_options(${PROJECT_NAME} PRIVATE -dynamiclib)
endif ()

target_link_libraries(${PROJECT_NAME} PRIVATE host_plugin ${AVCODEC_LIBRARY} ${AVUTIL_LIBRARY})

if (VA_LIBRARY)
    target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_VA_LIBRARY=1)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${VA_LIBRARY})
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(WARNING "VA library not found. Some functionality of the VAAPI encoders will be limited.")
endif ()

if (VULKAN_INCLUDE_DIR)
    include(CheckCXXSourceCompiles)

    set(CMAKE_REQUIRED_INCLUDES "${VULKAN_INCLUDE_DIR}")

    check_cxx_source_compiles("
    #include <vulkan/vulkan.h>

    int main() {
        VkVideoProfileInfoKHR profile{};
        VkVideoEncodeCapabilitiesKHR caps{};
        VkVideoEncodeAV1ProfileInfoKHR av1Profile{};
        return 0;
    }
    " HAVE_VULKAN_VIDEO)

    unset(CMAKE_REQUIRED_INCLUDES)

    if (HAVE_VULKAN_VIDEO)
        target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_VULKAN_LIBRARY=1)
    endif ()
endif ()

if ((CMAKE_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_SYSTEM_NAME STREQUAL "Windows")
        AND NOT HAVE_VULKAN_VIDEO)
    message(WARNING "Vulkan headers not found or are too old. Some functionality of the Vulkan encoders will be limited.")
endif ()

if (FFMPEG_IS_STATIC)
    target_link_libraries(${PROJECT_NAME}
            PRIVATE
            ${X264_LIBRARY}
            ${X265_LIBRARY}
            ${SVTAV1_LIBRARY}
            ${FFMPEG_SYSTEM_LIBS}
    )
endif ()
