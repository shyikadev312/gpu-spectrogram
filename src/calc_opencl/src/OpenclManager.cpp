#include <spectr/calc_opencl/OpenclManager.h>

#include <spectr/calc_opencl/OpenclUtils.h>

#include <spectr/utils/Asset.h>
#include <spectr/utils/Exception.h>

#include <spdlog/spdlog.h>

#include <sstream>

namespace spectr::calc_opencl
{
namespace
{
std::string toString(cl_device_type type)
{
    switch (type)
    {
        case CL_DEVICE_TYPE_DEFAULT: return "Default";
        case CL_DEVICE_TYPE_ALL: return "All";
        case CL_DEVICE_TYPE_CPU: return "CPU";
        case CL_DEVICE_TYPE_GPU: return "CPU";
        case CL_DEVICE_TYPE_ACCELERATOR: return "Accelerator";
        default: throw std::runtime_error("Invalid device type value.");
    }
}

std::string toString(const std::vector<cl_semaphore_payload_khr> values)
{
    std::stringstream ss;
    for (size_t i = 0; i < values.size(); ++i)
    {
        ss << std::to_string(values[i]);
        if (i != values.size() - 1)
        {
            ss << "x";
        }
    }
    return ss.str();
}

void openclCallback(const char* errinfo, const void* private_info, size_t cb, void* user_data)
{
    spdlog::error(errinfo);
}
}

OpenclManager::OpenclManager()
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
}

void OpenclManager::initContext(const std::vector<cl_context_properties>& additionalProperties)
{
    const cl_platform_id platformId = m_platform();

    std::vector<cl_context_properties> properties{
        CL_CONTEXT_PLATFORM,
        reinterpret_cast<cl_context_properties>(platformId),
    };
    properties.insert(properties.end(), additionalProperties.begin(), additionalProperties.end());
    properties.push_back(0);

    m_context = cl::Context(m_device, properties.data(), openclCallback, nullptr);
    // cl::Context::setDefault(m_context);
}

cl::Platform OpenclManager::getPlatform()
{
    ASSERT_MESSAGE(m_platform(), "Platform is nullptr.");
    return m_platform;
}

cl::Device OpenclManager::getDevice()
{
    ASSERT_MESSAGE(m_device(), "Device is nullptr.");
    return m_device;
}

cl::Context OpenclManager::getContext()
{
    ASSERT_MESSAGE(m_context(), "Context is nullptr.");
    return m_context;
}

}
