#pragma once

#include <spectr/calc_opencl/OpenclApi.h>

// #include <spdlog/spdlog.h>

#include <complex>
#include <iostream>
#include <sstream>
#include <vector>

namespace spectr::calc_opencl
{
#ifdef Complex // Complex is defined in X11/X.h
#undef Complex
#endif

using Complex = std::complex<float>;
static_assert(sizeof(Complex) == 2 * sizeof(cl_float));

class OpenclUtils
{
public:
    static void printPlatforms(std::ostream& out);

    static void printPlatform(const cl::Platform platform, std::ostream& out);

    static void printDevices(cl::Platform platform, std::ostream& out);

    static void printDevice(cl::Device device, std::ostream& out);

    static void printPlatformsAndDevices(std::ostream& out);

    static void printContextInfo(cl::Context context, std::ostream& out);

    static cl::Device getDevice(cl::Context context);

    template<typename T>
    static void printVector(cl::CommandQueue commandQueue,
                            cl::Buffer buffer,
                            size_t count,
                            const std::string& title = {})
    {
        std::vector<T> values;
        values.resize(count);
        cl::copy(commandQueue, buffer, values.begin(), values.end());

        std::stringstream ss;
        ss << (title.empty() ? "Values:" : title.c_str());
        ss << "\n";
        for (const auto& value : values)
        {
            ss << value << "\n";
        }

        // spdlog::debug(ss.str());
        std::cout << ss.str() << std::endl;
    }

    static void printComplexNumbers(cl::CommandQueue commandQueue,
                                    cl::Buffer buffer,
                                    size_t complexNumbersCount);
};
}
