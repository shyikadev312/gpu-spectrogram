#pragma once

#include <spectr/calc_opencl/OpenclApi.h>

#include <iostream>

namespace spectr::calc_opencl
{
class OpenclUtils
{
public:
    static void printPlatforms(std::ostream& out);

    static void printPlatform(cl::Platform platform, std::ostream& out);

    static void printDevices(cl::Platform platform, std::ostream& out);

    static void printDevice(cl::Device device, std::ostream& out);

    static void printPlatformsAndDevices(std::ostream& out);

    static void printContextInfo(cl::Context context, std::ostream& out);
};
}
