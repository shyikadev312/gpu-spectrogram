#pragma once

// Suppress Visual Studio warnings for OpenCL library code
#ifdef _MSC_VER
#define _SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING
#define _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS
#endif // _MSC_VER

#define CL_HPP_TARGET_OPENCL_VERSION 200

#define CL_HPP_ENABLE_EXCEPTIONS

#include <CL/opencl.hpp>

#include <spectr/utils/Assert.h>

#define ASSERT_CL_SUCCESS(expr) ASSERT(expr == CL_SUCCESS);
