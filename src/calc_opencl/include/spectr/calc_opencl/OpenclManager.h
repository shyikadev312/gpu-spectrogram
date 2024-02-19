#pragma once

#include <spectr/calc_opencl/OpenclApi.h>

namespace spectr::calc_opencl
{
/**
 * @brief OpenCL manager chooses OpenCL device, creates and initialises OpenCL context.
 */
class OpenclManager
{
public:
    OpenclManager(const std::vector<cl_context_properties>& additionalProperties = {});

    cl::Platform getPlatform() const;

    cl::Device getDevice() const;

    cl::Context getContext() const;

private:
    cl::Platform m_platform;
    cl::Device m_device;
    cl::Context m_context;
};
}
