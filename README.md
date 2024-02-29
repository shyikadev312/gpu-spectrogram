NUAND CONFIDENTIAL AND PROPRIETY
ALL RIGHTS RESERVED


This repository belongs to the Nuand GPU accelerated spectogram project.

## Table Of Contents

- [Supported platforms](#Supported-platforms)
- [How to build](#How-to-build)
    - [How to build on Windows](#How-to-build-on-Windows)
    - [How to build on Linux](#How-to-build-on-Linux)
- [Project structure](#Project-structure)
- [Tips and tricks](#Tips-and-tricks)
- [Third-party licenses](#Third-party-licenses)

## Supported platforms

The following OS are supported:
* Windows
* Linux

The following OS are verified:
* Windows 10
* Windows 11
* Ubuntu 22.04
* Ubuntu 24.04

The following architectures are supported:
* AMD64

The following compilers are supported:
* C++20 compilers (msvc, g++, clang)

## How to build

### How to build on Windows

Required software:

* C++20 compiler (Visual Studio 2022)
* Latest graphics card driver installed (or latest OpenCL driver installed)

Compilation steps:

1. Open `GPU Spectrogram.sln` in Visual Studio 2022
2. Ensure the correct build profile is set (either `Debug x64` or `Release x64`)
3. Build (Go to `Build` -> `Build Solution` or press `F7`)

### How to build on Linux

Required software:

* C++20 compiler (g++, clang)
* CMake
* Python3
* Conan 1.x (1.62 version is verified. DON'T INSTALL Conan major version 2)
* Latest graphics card driver installed (or latest OpenCL driver installed)

Shell commands to install the required software:

```shell
sudo apt update

sudo apt install git g++ clang cmake python3 python3-pip clang-format pkg-config -y

sudo apt install nvidia-opencl-dev -y

sudo apt install libgl1-mesa-dev libx11-xcb-dev libfontenc-dev libxcb-cursor-dev libice-dev libsm-dev libxaw7-dev libxcomposite-dev libxcursor-dev libxdamage-dev libxext-dev libxfixes-dev libxi-dev libxinerama-dev libxkbfile-dev libxmu-dev libxmuu-dev libxpm-dev libxrandr-dev libxrender-dev libxres-dev libxss-dev libxt-dev libxtst-dev libxv-dev libxvmc-dev libxxf86vm-dev libxcb-xkb-dev libxcb-icccm4-dev libxcb-keysyms1-dev libxcb-randr0-dev libxcb-shape0-dev libxcb-sync-dev libxcb-xfixes0-dev libxcb-xinerama0-dev libxcb-dri3-dev uuid-dev libxcb-util-dev libxcb-util0-dev -y

sudo apt upgrade -y
```

Install Conan package manager. It is required to install exactly major version 1.
Don't install Conan version 2, it has some breaking changes, and these instructions won't work.

```shell
sudo pip install conan==1.62
```

If you get the next warning from Conan: "WARNING: GCC OLD ABI COMPATIBILITY", use these commands to set the new version:
```shell
conan profile new default --detect
conan profile update settings.compiler.libcxx=libstdc++11 default
```

How to build from command line:
```shell
cd gpu-spectrogram
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

## Project structure

Project structure and architecture are described in the [Architecture section](./docs/Architecture.md).

## Problems, tips and tricks

* Use Conan 1.x,. Don't install Conan 2.0, it's not fully stable yet.
* Ensure that you have a powerful discrete GPU with the newest drivers installed.
* Ensure that your GPU has an OpenCL support.
* If you have poor performance on Windows+Nvidia environment, try to disable VSync in Nvidia control panel. There is an issue with OpenCL+OpenGL interop freeze when calling enqueueAcquireGLObjects().

## Third-party licenses

* GLFW - https://github.com/glfw/glfw - cross-platform window/input framework - Zlib License.
* fmt - https://github.com/fmtlib/fmt - formatting library for logging - unnamed permissive license.
* glm - https://github.com/g-truc/glm - 3d math library - The Happy Bunny License (Modified MIT License).
* spdlog - https://github.com/gabime/spdlog - logging library - MIT license.
* GTest - https://github.com/google/googletest - Google test framework - BSD-3-Clause license.
* glad - https://github.com/Dav1dde/glad - for loading OpenGL functions - MIT license.
* imgui - https://github.com/ocornut/imgui - for fast GUI rendering - MIT License.
* Intel One Mono font - https://github.com/intel/intel-one-mono - for GUI rendering - OFL-1.1 license.
* Google Benchmark - https://github.com/google/benchmark - benchmarking library - Apache-2.0 license.
* OpenCL headers - https://github.com/KhronosGroup/OpenCL-Headers - OpenCL API - Apache-2.0 license.
* OpenCL Cpp headers - https://github.com/KhronosGroup/OpenCL-CLHPP - OpenCL C++ API - Apache-2.0 license.
* OpenCL ISD loader - https://github.com/KhronosGroup/OpenCL-ICD-Loader - OpenCL driver loader - Apache-2.0 license.
* Boost - https://www.boost.org/ - extensions of C++ standard library - Boost Software License.

All licenses are permissive and their usage is allowed for commercial use with the copyright notice provided.
