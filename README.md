# FFmpeg Encoder Plugin for DaVinci Resolve Studio

This plugin enables you to export H.264, H.265 and AV1 video from DaVinci Resolve Studio using FFmpeg encoders.
The plugin supports both CPU encoding and GPU encoding on Linux using VAAPI.

### Supported encoders
- H.264
  - X264
  - VAAPI
  - NVENC
- H.265
  - X265
  - VAAPI
  - NVENC
- AV1
  - SVT-AV1
  - VAAPI
  - NVENC

## Installation

### Linux
- Unzip [ffmpeg_encoder_plugin.dvcp.bundle.zip](https://github.com/EdvinNilsson/ffmpeg_encoder_plugin/releases/latest/download/ffmpeg_encoder_plugin.dvcp.bundle.zip) to `/opt/resolve/IOPlugins/`.
- Install FFmpeg using the package manager (e.g. `sudo apt install ffmpeg`, `sudo pacman -S ffmpeg`).

There is also an AUR package at [davinci-ffmpeg-encoder-plugin](https://aur.archlinux.org/packages/davinci-ffmpeg-encoder-plugin) for Arch Linux and Arch-based distributions.

If the hardware-accelerated VAAPI encoders are missing in Resolve, you may lack the required VAAPI driver for your GPU. Installation instructions can be found on the wikis of [Arch](https://wiki.archlinux.org/title/Hardware_video_acceleration#Installation), [Debian](https://wiki.debian.org/HardwareVideoAcceleration#VA-API), and [Fedora](https://fedoraproject.org/wiki/Hardware_Video_Acceleration#VA-API). The driver installation can be verified by running `vainfo`.

###  Windows
- Unzip [ffmpeg_encoder_plugin.dvcp.bundle.zip](https://github.com/EdvinNilsson/ffmpeg_encoder_plugin/releases/latest/download/ffmpeg_encoder_plugin.dvcp.bundle.zip) to `%ProgramData%\Blackmagic Design\DaVinci Resolve\Support\IOPlugins\`.

### Mac
See build instructions below.

## Build instructions

### Linux

#### Requirements

- GCC or Clang
- CMake
- FFmpeg (with development files)

#### Building the plugin

```bash
mkdir build
cd build
cmake ..
make
```
- Copy `ffmpeg_encoder_plugin.dvcp` from `./build` to `/opt/resolve/IOPlugins/ffmpeg_encoder_plugin.dvcp.bundle/Contents/Linux-x86-64/`.

### Windows

#### Requirements

- Visual Studio 2022
- MSVC or Clang
- CMake
- Shared FFmpeg development files from https://www.gyan.dev/ffmpeg/builds/

#### Building the plugin
- Extract the downloaded `ffmpeg-release-full-shared.7z` to the project root, and rename the folder to `ffmpeg`.
- Open the project in Visual Studio 2022 and select Build > Build ffmpeg_encoder_plugin.dvcp.
- Copy `ffmpeg_encoder_plugin.dvcp` from `.\out\build`, and `avcodec-61.dll`, `avutil-59.dll` and `swresample-5.dll` from `.\ffmpeg\bin` to `%ProgramData%\Blackmagic Design\DaVinci Resolve\Support\IOPlugins\ffmpeg_encoder_plugin.dvcp.bundle\Contents\Win64\`.

### Mac

#### Requirements

- Clang
- CMake (from Homebrew)
- FFmpeg (from Homebrew)

#### Building the plugin

```bash
mkdir build
cd build
cmake ..
make
```
- Copy `ffmpeg_encoder_plugin.dvcp` from `./build` to the following directory depending on the CPU architecture and installation location of DaVinci Resolve.
  - Mac x86-64: `/Library/Application Support/Blackmagic Design/DaVinci Resolve/IOPlugins/ffmpeg_encoder_plugin.dvcp.bundle/Contents/MacOS-x86-64/`
  - Mac x86-64 (App Store): `~/Library/Containers/com.blackmagic-design.DaVinciResolveAppStore/Data/Library/Application Support/IOPlugins/ffmpeg_encoder_plugin.dvcp.bundle/Contents/MacOS-x86-64/`
  - Mac Arm64: `/Library/Application Support/Blackmagic Design/DaVinci Resolve/IOPlugins/ffmpeg_encoder_plugin.dvcp.bundle/Contents/MacOS/`
  - Mac Arm64 (App Store): `~/Library/Containers/com.blackmagic-design.DaVinciResolveAppStore/Data/Library/Application Support/IOPlugins/ffmpeg_encoder_plugin.dvcp.bundle/Contents/MacOS/`

<sub>DaVinci Resolve is a trademark of Blackmagic Design Pty. Ltd.</sub>
