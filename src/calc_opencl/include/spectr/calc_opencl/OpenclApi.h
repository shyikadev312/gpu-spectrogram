#pragma once

#define CL_HPP_TARGET_OPENCL_VERSION 200

#define CL_HPP_ENABLE_EXCEPTIONS

#include <CL/opencl.hpp>

#include <spectr/utils/Assert.h>

#define ASSERT_CL_SUCCESS(expr) ASSERT(expr == CL_SUCCESS);
