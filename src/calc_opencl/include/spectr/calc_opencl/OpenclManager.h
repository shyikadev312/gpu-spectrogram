#pragma once

#include <spectr/calc_opencl/OpenClApi.h>

namespace spectr::calc_opencl
{
class OpenclManager
{
public:
    OpenclManager();

    void initContext(const std::vector<cl_context_properties>& additionalProperties = {});

    cl::Platform getPlatform();

    cl::Device getDevice();

    cl::Context getContext();

private:
    cl::Platform m_platform{};
    cl::Device m_device{};
    cl::Context m_context{};
};
}
