cmake_minimum_required(VERSION 3.20)
project(host_plugin)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(BASE_DIR "${CMAKE_SOURCE_DIR}/../")

include_directories(${BASE_DIR}/include)

add_library(${PROJECT_NAME} STATIC host_api.cpp host_api.h plugin_api.cpp plugin_api.h)

if (NOT MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE -fPIC -Wall -Wno-unused-variable -Wno-multichar)
endif ()
