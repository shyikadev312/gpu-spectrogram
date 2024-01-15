NUAND CONFIDENTIAL AND PROPRIETY
ALL RIGHTS RESERVED


This repository belongs to the Nuand GPU accelerated spectogram project.

## Table Of Contents

- [Supported platforms](#Supported-platforms)
- [How to build](#How-to-build)
    - [How to build on Windows](#How-to-build-on-Windows)
    - [How to build on Linux](#How-to-build-on-Linux)
- [Third-party licenses](#Third-party-licenses)
- [Project structure](#Project-structure)

## Supported platforms

The next OS are supported:
* Windows
* Linux

The next OS are verified:
* Windows 11
* Ubuntu 22.04

The next architectures are supported:
* AMD64

The next compilers are supported:
* C++17 compilers (msvc, g++, clang of corresponding version)

## How to build

### How to build on Windows

Required software:

* C++17 compiler (Visual Studio 2022)
* CMake (latest version is recommended)
* Python3 (latest version is recommended)
* Conan 1.x (1.62 version is verified. Don't install Conan major version 2)

Install Conan package manager. It is required to install exactly major version 1.
Don't install Conan version 2, it has some breaking changes, and these instructions won't work.

```shell
sudo pip install conan==1.62
```

How to build from command line:

```cmd
cd gpu-spectrogram
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

### How to build on Linux

Required software:

* C++17 compiler (g++, clang)
* CMake
* Python3
* Conan 1.x (1.62 version is verified. Don't install Conan major version 2)

Shell commands to install the required software:

```shell
sudo apt update

sudo apt install git g++ clang cmake python3 python3-pip clang-format pkg-config -y

sudo apt install nvidia-opencl-dev -y

sudo apt install libgl1-mesa-dev libx11-xcb-dev libfontenc-dev libxcb-cursor-dev libice-dev libsm-dev libxaw7-dev libxcomposite-dev libxcursor-dev libxdamage-dev libxext-dev libxfixes-dev libxi-dev libxinerama-dev libxkbfile-dev libxmu-dev libxmuu-dev libxpm-dev libxrandr-dev libxrender-dev libxres-dev libxss-dev libxt-dev libxtst-dev libxv-dev libxvmc-dev libxxf86vm-dev libxcb-xkb-dev libxcb-icccm4-dev libxcb-keysyms1-dev libxcb-randr0-dev libxcb-shape0-dev libxcb-sync-dev libxcb-xfixes0-dev libxcb-xinerama0-dev libxcb-dri3-dev uuid-dev libxcb-util-dev libxcb-util0-dev -y

sudo apt update
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

All source code files are placed under "./src" dir. The project is splitted into bunch of submodules that are built into libs and executables.

All assets are placed under "./assets" dir. It includes shaders for OpenGL, kernels for OpenCL, fonts for ImGUI, audio samples for debugging.

List of the project modules (libs):
* spectr.utils - Set of some utility methods and classes.
* spectr.audio_loader - Audio loader that can load WAV audio files as raw sample buffers.
* spectr.calc_cpu - Implementation of calculations on CPU.
* spectr.calc_opencl - Implementation of calculations on GPU with OpenCL.
* spectr.render_gl - Implementation of rendering with OpenGL.
* spectr.desktop_app - Main executable, application for rendering the waterfall diagram.

## Tips and tricks

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

All licenses are permissive and their usage is allowed for commercial use with the copyright notice provided.
