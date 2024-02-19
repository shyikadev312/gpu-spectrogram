#include <spectr/calc_opencl/OpenclManager.h>

#include <spectr/calc_opencl/OpenclUtils.h>

#include <spectr/utils/Exception.h>

#include <spdlog/spdlog.h>

#include <sstream>

namespace spectr::calc_opencl
{
namespace
{
void notifyCallback(const char* errorInfo, const void* privateInfo, size_t cb, void* userData)
{
    spdlog::error(errorInfo);
}
}

OpenclManager::OpenclManager(const std::vector<cl_context_properties>& additionalProperties)
{
    std::stringstream ss;
    OpenclUtils::printPlatformsAndDevices(ss);
    spdlog::trace(ss.str());

    std::vector<cl::Platform> platforms;
    ASSERT_CL_SUCCESS(cl::Platform::get(&platforms));

    if (platforms.empty())
    {
        throw utils::Exception("No platforms found for OpenCL!");
    }

    std::vector<std::pair<cl::Platform, cl::Device>> candidates;
    for (const auto& platform : platforms)
    {
        std::vector<cl::Device> devices;
        ASSERT_CL_SUCCESS(platform.getDevices(CL_DEVICE_TYPE_ALL, &devices));
        if (devices.empty())
        {
            throw utils::Exception("No devices found.");
        }

        for (const auto& device : devices)
        {
            const auto type = device.getInfo<CL_DEVICE_TYPE>();
            if (type & CL_DEVICE_TYPE_GPU)
            {
                candidates.push_back({ platform, device });
            }
        }
    }

    if (candidates.empty())
    {
        throw utils::Exception("Failed to select device.");
    }

    const auto& selected = candidates.front();
    m_platform = selected.first;
    m_device = selected.second;

    const cl_platform_id platformId = m_platform();

    std::vector<cl_context_properties> properties{
        CL_CONTEXT_PLATFORM,
        reinterpret_cast<cl_context_properties>(platformId),
    };
    properties.insert(properties.end(), additionalProperties.begin(), additionalProperties.end());
    properties.push_back(0);

    m_context = cl::Context(m_device, properties.data(), notifyCallback);
}

cl::Platform OpenclManager::getPlatform() const
{
    ASSERT_MESSAGE(m_platform(), "Platform is nullptr.");
    return m_platform;
}

cl::Device OpenclManager::getDevice() const
{
    ASSERT_MESSAGE(m_device(), "Device is nullptr.");
    return m_device;
}

cl::Context OpenclManager::getContext() const
{
    ASSERT_MESSAGE(m_context(), "Context is nullptr.");
    return m_context;
}
}
