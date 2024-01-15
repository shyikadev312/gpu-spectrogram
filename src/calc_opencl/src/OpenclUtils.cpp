#include <spectr/calc_opencl/OpenclUtils.h>

#include <iomanip>
#include <sstream>
#include <vector>

namespace spectr::calc_opencl
{
namespace
{
std::string toStringType(cl_device_type deviceType)
{
    switch (deviceType)
    {
        case CL_DEVICE_TYPE_DEFAULT: return "Default";
        case CL_DEVICE_TYPE_CPU: return "CPU";
        case CL_DEVICE_TYPE_GPU: return "GPU";
#if defined(CL_VERSION_1_2)
        case CL_DEVICE_TYPE_ACCELERATOR: return "Accelerator";
#endif
        case CL_DEVICE_TYPE_ALL: return "All";
        default: throw std::runtime_error("Not recognized device type.");
    }
}

std::string toStringSize(const cl::vector<cl::size_type> values)
{
    std::stringstream ss;
    for (size_t i = 0; i < values.size(); ++i)
    {
        const auto& value = values[i];
        ss << value;
        if (i + 1 != values.size())
        {
            ss << "x";
        }
    }
    return ss.str();
}

std::string toStringBool(cl_bool val)
{
    switch (val)
    {
        case CL_TRUE: return "true";
        case CL_FALSE: return "false";
        default: throw std::runtime_error("Unrecognized cl_bool values.");
    }
}

#if defined(CL_VERSION_3_0)
std::string toStringAtomicCapabilities(cl_device_atomic_capabilities val)
{
    std::stringstream ss;

    if (val & CL_DEVICE_ATOMIC_ORDER_RELAXED)
    {
        ss << "\tSupport for the relaxed memory order.\n";
    }

    if (val & CL_DEVICE_ATOMIC_ORDER_ACQ_REL)
    {
        ss << "\tSupport for the acquire, release, and acquire - release memory orders.\n";
    }

    if (val & CL_DEVICE_ATOMIC_ORDER_SEQ_CST)
    {
        ss << "\tSupport for the sequentially consistent memory order.\n";
    }

    if (val & CL_DEVICE_ATOMIC_SCOPE_WORK_ITEM)
    {
        ss << "\tSupport for memory ordering constraints that apply to a single work-item.\n";
    }

    if (val & CL_DEVICE_ATOMIC_SCOPE_WORK_GROUP)
    {
        ss << "\tSupport for memory ordering constraints that apply to all work-items in a work - "
              "group.\n";
    }

    if (val & CL_DEVICE_ATOMIC_SCOPE_DEVICE)
    {
        ss << "\tSupport for memory ordering constraints that apply to all work-items executing on "
              "the device.\n";
    }

    if (val & CL_DEVICE_ATOMIC_SCOPE_ALL_DEVICES)
    {
        ss
          << "\tSupport for memory ordering constraints that apply to all work-items executing "
             "across all devices that can share SVM memory with each other and the host process.\n";
    }

    return ss.str();
}
#endif

std::string toStringFpConfig(cl_device_fp_config val)
{
    std::stringstream ss;

    if (val & CL_FP_DENORM)
    {
        ss << "\tdenorms are supported;\n";
    }

    if (val & CL_FP_INF_NAN)
    {
        ss << "\tINF and quiet NaNs are supported;\n";
    }

    if (val & CL_FP_ROUND_TO_NEAREST)
    {
        ss << "\tround to nearest even rounding mode supported;\n";
    }

    if (val & CL_FP_ROUND_TO_ZERO)
    {
        ss << "\tround to zero rounding mode supported;\n";
    }

    if (val & CL_FP_ROUND_TO_INF)
    {
        ss << "\tround to positive and negative infinity rounding modes supported;\n";
    }

    if (val & CL_FP_FMA)
    {
        ss << "\tIEEE754-2008 fused multiply-add is supported;\n";
    }

    if (val & CL_FP_CORRECTLY_ROUNDED_DIVIDE_SQRT)
    {
        ss << "\tdivide and sqrt are correctly rounded as defined by the IEEE754 specification;\n";
    }

    if (val & CL_FP_SOFT_FLOAT)
    {
        ss << "\tbasic floating-point operations (such as addition, subtraction, multiplication) "
              "are "
              "implemented in software;\n";
    }

    return ss.str();
}

std::string toStringMemCacheType(cl_device_mem_cache_type val)
{
    switch (val)
    {
        case CL_NONE: return "none";
        case CL_READ_ONLY_CACHE: return "read-only cache";
        case CL_READ_WRITE_CACHE: return "read-write cache";
        default: throw std::runtime_error("Unsupported value.");
    }
}

constexpr auto BytesInKb = 1 << 10;
constexpr auto BytesInMb = 1 << 20;

std::string toMb(size_t bytes)
{

    const auto mb = static_cast<float>(bytes) / BytesInMb;
    std::stringstream ss;
    ss << std::setprecision(2) << mb << " MB";
    return ss.str();
}

std::string toStringMemory(cl_ulong bytes)
{
    std::stringstream ss;

    if (bytes <= 10 * BytesInKb)
    {
        ss << bytes << " bytes";
    }
    else
    {
        ss << toMb(bytes);
    }

    return ss.str();
}

std::string toStringLocalMemType(cl_device_local_mem_type localMemType)
{
    switch (localMemType)
    {
        case CL_LOCAL: return "local";
        case CL_GLOBAL: return "global";
        case CL_NONE: return "none";
        default: throw std::runtime_error("Unsupported value.");
    }
}

#if defined(CL_VERSION_3_0)
std::string toStringNameVersions(const std::vector<cl_name_version>& nameVersions)
{
    std::stringstream ss;
    for (const auto& data : nameVersions)
    {
        ss << "\tName: " << data.name << " Version: " << data.version << "\n";
    }
    return ss.str();
}
#endif

std::string toStringProperty(cl_uint propertyName, intptr_t propertyValue)
{
    std::stringstream ss;

    switch (propertyName)
    {
        case CL_CONTEXT_PLATFORM:
        {
            const cl_platform_id platform = reinterpret_cast<cl_platform_id>(propertyValue);
            ss << "Context platform: " << propertyValue << "\n";
            break;
        }

        case CL_GL_CONTEXT_KHR:
        {
            ss << "GL Context: " << propertyValue << "\n";
            break;
        }

        case CL_WGL_HDC_KHR:
        {
            ss << "WGL HDC: " << propertyValue << "\n";
            break;
        }

        default: throw std::runtime_error("Unsupported context property name.");
    }

    return ss.str();
}
}

void OpenclUtils::printPlatforms(std::ostream& out)
{
    cl::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);

    for (const auto& platform : platforms)
    {
        printPlatform(platform, out);
    }
}

void OpenclUtils::printPlatform(cl::Platform platform, std::ostream& out)
{
    const auto profile = platform.getInfo<CL_PLATFORM_PROFILE>();
    out << "Profile: " << profile << "\n";

    const auto version = platform.getInfo<CL_PLATFORM_VERSION>();
    out << "Version: " << version << "\n";

#if defined(CL_VERSION_3_0)
    const auto numericVersion = platform.getInfo<CL_PLATFORM_NUMERIC_VERSION>();
    out << "Numeric version: " << numericVersion << "\n";
#endif

    const auto name = platform.getInfo<CL_PLATFORM_NAME>();
    out << "Name: " << name << "\n";

    const auto vendor = platform.getInfo<CL_PLATFORM_VENDOR>();
    out << "Vendor: " << vendor << "\n";

    const auto extensions = platform.getInfo<CL_PLATFORM_EXTENSIONS>();
    out << "Extensions: " << extensions << "\n";

#if defined(CL_VERSION_3_0)
    const auto extensionsWithVersion = platform.getInfo<CL_PLATFORM_EXTENSIONS_WITH_VERSION>();
    out << "Extensions with version:\n";
    for (const auto& extensionWithVersion : extensionsWithVersion)
    {
        out << "Name: " << extensionWithVersion.name << " Version: " << extensionWithVersion.version
            << "\n";
    }
#endif

#if defined(CL_VERSION_2_1)
    const auto hostTimeResolution = platform.getInfo<CL_PLATFORM_HOST_TIMER_RESOLUTION>();
    out << "Host timer resolution: " << hostTimeResolution << " nanoseconds\n";
#endif
}

void OpenclUtils::printDevices(cl::Platform platform, std::ostream& out)
{
    cl::vector<cl::Device> devices;
    platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);
    for (const auto& device : devices)
    {
        out << "---------------------------------------\n";
        printDevice(device, out);
    }
}

void OpenclUtils::printDevice(cl::Device device, std::ostream& out)
{
    {
        const auto type = device.getInfo<CL_DEVICE_TYPE>();
        out << "Type: " << toStringType(type) << "\n";
    }

    {
        const auto vendorId = device.getInfo<CL_DEVICE_VENDOR_ID>();
        out << "Vendor Id: " << vendorId << "\n";
    }

    {
        const auto maxComputeUnits = device.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>();
        out << "Max compute units: " << maxComputeUnits << "\n";
    }

    {
        const auto maxWorkItemDimensions = device.getInfo<CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS>();
        out << "Max work item dimensions: " << maxWorkItemDimensions << "\n";
    }

    {
        const auto maxWorkItemSizes = device.getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>();
        out << "Max work item sizes: " << toStringSize(maxWorkItemSizes) << "\n";
    }

    {
        const auto maxWorkGroupSize = device.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>();
        out << "Max work group size: " << maxWorkGroupSize << "\n";
    }

    {
        const auto preferredVectorWidthChar =
          device.getInfo<CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR>();
        out << "Preferred vector width char: " << preferredVectorWidthChar << "\n";
    }

    {
        const auto preferredVectorWidthShort =
          device.getInfo<CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT>();
        out << "Preferred vector width short: "
            << device.getInfo<CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT>() << "\n";
    }

    {
        const auto preferredVectorWidthInt = device.getInfo<CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT>();
        out << "Preferred vector width int: " << preferredVectorWidthInt << "\n";
    }

    {
        const auto preferredVectorWidthLong =
          device.getInfo<CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG>();
        out << "Preferred vector width long: " << preferredVectorWidthLong << "\n";
    }

    {
        const auto preferredVectorWidthFloat =
          device.getInfo<CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT>();
        out << "Preferred vector width float: " << preferredVectorWidthFloat << "\n";
    }

    {
        const auto preferredVectorWidthDouble =
          device.getInfo<CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE>();
        out << "Preferred vector width double: " << preferredVectorWidthDouble << "\n";
    }

    {
#if CL_TARGET_OPENCL_VERSION >= 110
        const auto preferredVectorWidthHalf =
          device.getInfo<CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF>();
        out << "Preferred vector width half: " << preferredVectorWidthHalf << "\n";
#endif
    }

    {
        // todo CL_DEVICE_NATIVE_VECTOR_WIDTH
    }

    {
        const auto maxClockFrequency = device.getInfo<CL_DEVICE_MAX_CLOCK_FREQUENCY>();
        out << "Max clock frequency: " << maxClockFrequency << "\n";
    }

    {
        const auto addressBits = device.getInfo<CL_DEVICE_ADDRESS_BITS>();
        out << "Address bits: " << addressBits << "\n";
    }

    {
        const auto maxMemAllocSize = device.getInfo<CL_DEVICE_MAX_MEM_ALLOC_SIZE>();
        out << "Max memory allocation size: " << toStringMemory(maxMemAllocSize) << "\n";
    }

    {
        const auto imageSupport = device.getInfo<CL_DEVICE_IMAGE_SUPPORT>();
        out << "Max image support: " << imageSupport << "\n";
    }

    {
        const auto maxReadImageArgs = device.getInfo<CL_DEVICE_MAX_READ_IMAGE_ARGS>();
        out << "Max read image args: " << maxReadImageArgs << "\n";
    }

    {
        const auto maxWriteImageArgs = device.getInfo<CL_DEVICE_MAX_WRITE_IMAGE_ARGS>();
        out << "Max write image args: " << maxWriteImageArgs << "\n";
    }

#if defined(CL_VERSION_2_0)
    {
        const auto maxReadWriteImageArgs = device.getInfo<CL_DEVICE_MAX_READ_WRITE_IMAGE_ARGS>();
        out << "Max read write image args: " << maxReadWriteImageArgs << "\n";
    }
#endif

#if defined(CL_VERSION_2_1)
    {
        const auto ilVersion = device.getInfo<CL_DEVICE_IL_VERSION>();
        out << "IL version: " << ilVersion << "\n";
    }
#endif

#if defined(CL_VERSION_3_0)
    {

        const auto ilsWithVersion = device.getInfo<CL_DEVICE_ILS_WITH_VERSION>();
        out << "IL versions list:\n" << toStringNameVersions(ilsWithVersion);
    }
#endif

    {
        const auto image2dMaxWidth = device.getInfo<CL_DEVICE_IMAGE2D_MAX_WIDTH>();
        const auto image2dMaxHeight = device.getInfo<CL_DEVICE_IMAGE2D_MAX_HEIGHT>();
        out << "Image 2d max size: " << image2dMaxWidth << "x" << image2dMaxHeight << "\n";
    }

    {
        const auto image3dMaxWidth = device.getInfo<CL_DEVICE_IMAGE3D_MAX_WIDTH>();
        const auto image3dMaxHeight = device.getInfo<CL_DEVICE_IMAGE3D_MAX_HEIGHT>();
        const auto image3dMaxDepth = device.getInfo<CL_DEVICE_IMAGE3D_MAX_DEPTH>();
        out << "Image 3d max size: " << image3dMaxWidth << "x" << image3dMaxHeight << "x"
            << image3dMaxDepth << "\n";
    }

#if defined(CL_VERSION_1_2)
    {
        const auto imageMaxBufferSize = device.getInfo<CL_DEVICE_IMAGE_MAX_BUFFER_SIZE>();
        out << "Image max buffer size: " << imageMaxBufferSize << "\n";
    }
#endif

#if defined(CL_VERSION_1_2)
    {
        const auto imageMaxArraySize = device.getInfo<CL_DEVICE_IMAGE_MAX_ARRAY_SIZE>();
        out << "Image max array size: " << imageMaxArraySize << "\n";
    }
#endif

    const auto maxSamplers = device.getInfo<CL_DEVICE_MAX_SAMPLERS>();
    out << "Max samplers number: " << maxSamplers;

#if defined(CL_VERSION_2_0)
    {
        const auto imagePitchAlignment = device.getInfo<CL_DEVICE_IMAGE_PITCH_ALIGNMENT>();
        out << "Image pitch alignment: " << imagePitchAlignment << "\n";
    }
#endif

#if defined(CL_VERSION_2_0)
    {
        const auto imageBaseAddressAlignment =
          device.getInfo<CL_DEVICE_IMAGE_BASE_ADDRESS_ALIGNMENT>();
        out << "Image base address alignment: " << imageBaseAddressAlignment << "\n";
    }
#endif

#if defined(CL_VERSION_2_0)
    {
        const auto maxPipeArgs = device.getInfo<CL_DEVICE_MAX_PIPE_ARGS>();
        out << "Max pipe args: " << maxPipeArgs << "\n";
    }
#endif

#if defined(CL_VERSION_2_0)
    {
        const auto pipeMaxActiveReservations =
          device.getInfo<CL_DEVICE_PIPE_MAX_ACTIVE_RESERVATIONS>();
        out << "Pipe max active reservations: " << pipeMaxActiveReservations << "\n";
    }
#endif

#if defined(CL_VERSION_2_0)
    {
        const auto pipeMaxPacketSize = device.getInfo<CL_DEVICE_PIPE_MAX_PACKET_SIZE>();
        out << "Pipe max packet size: " << pipeMaxPacketSize << "\n";
    }
#endif

    {
        const auto maxParameterSize = device.getInfo<CL_DEVICE_MAX_PARAMETER_SIZE>();
        out << "Max parameter size: " << maxParameterSize << " bytes\n";
    }

    {
        const auto memBaseAddrAlign = device.getInfo<CL_DEVICE_MEM_BASE_ADDR_ALIGN>();
        out << "Mem base address alignment: " << memBaseAddrAlign << "\n";
    }

    //  CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE - deprecated

    {
        const auto singleFpConfig = device.getInfo<CL_DEVICE_SINGLE_FP_CONFIG>();
        out << "Single floating-point config:\n" << toStringFpConfig(singleFpConfig) << "\n";
    }

#if defined(CL_VERSION_1_2)
    {
        const auto doubleFpConfig = device.getInfo<CL_DEVICE_DOUBLE_FP_CONFIG>();
        out << "Double floating-point config:\n" << toStringFpConfig(doubleFpConfig) << "\n";
    }
#endif

    {
        const auto globalMemCacheType = device.getInfo<CL_DEVICE_GLOBAL_MEM_CACHE_TYPE>();
        out << "Global memory cache type: " << toStringMemCacheType(globalMemCacheType) << "\n";
    }

    {
        const auto globalMemCachelineSize = device.getInfo<CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE>();
        out << "Global memory cache line in bytes: " << globalMemCachelineSize << "\n";
    }

    {
        const auto globalMemCacheSize = device.getInfo<CL_DEVICE_GLOBAL_MEM_CACHE_SIZE>();
        out << "Global memory cache size: " << globalMemCacheSize << " bytes\n";
    }

    {
        const auto globalMemSize = device.getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>();
        out << "Size of global device memory: " << globalMemSize << " bytes.\n";
    }

    {
        const auto maxConstantBufferSize = device.getInfo<CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE>();
        out << "Max size in bytes of a constant buffer allocation: "
            << toStringMemory(maxConstantBufferSize) << "\n";
    }

    {
        const auto maxConstantArgs = device.getInfo<CL_DEVICE_MAX_CONSTANT_ARGS>();
        out << "Max global variable size: " << toStringMemory(maxConstantArgs) << "\n";
    }

#if defined(CL_VERSION_2_0)
    {
        const auto maxGlobalVariableSize = device.getInfo<CL_DEVICE_MAX_GLOBAL_VARIABLE_SIZE>();
        out << "Max global variable size: " << toStringMemory(maxGlobalVariableSize) << "\n";
    }
#endif

#if defined(CL_VERSION_2_0)
    {
        const auto globalVariablePreferredTotalSize =
          device.getInfo<CL_DEVICE_GLOBAL_VARIABLE_PREFERRED_TOTAL_SIZE>();
        out << "Max preferred size of all program variable in global address space: "
            << toStringMemory(globalVariablePreferredTotalSize) << "\n";
    }
#endif

    {
        const auto localMemType = device.getInfo<CL_DEVICE_LOCAL_MEM_TYPE>();
        out << "Type of local memory supported: " << toStringLocalMemType(localMemType) << "\n";
    }

    {
        const auto localMemSize = device.getInfo<CL_DEVICE_LOCAL_MEM_SIZE>();
        out << "Local memory size: " << toStringMemory(localMemSize) << "\n";
    }

    {
        const auto errorCorrectionSupport = device.getInfo<CL_DEVICE_ERROR_CORRECTION_SUPPORT>();
        out << "Error correction for memory access supported: "
            << toStringBool(errorCorrectionSupport) << "\n";
    }

    // CL_DEVICE_HOST_UNIFIED_MEMORY - deprecated by 2.0

    {
        const auto profilingTimerResolution =
          device.getInfo<CL_DEVICE_PROFILING_TIMER_RESOLUTION>();
        out << "Profiling timer resolution: " << profilingTimerResolution << " nanoseconds\n";
    }

    {
        const auto endianLittle = device.getInfo<CL_DEVICE_ENDIAN_LITTLE>();
        out << "Endian little: " << toStringBool(endianLittle) << "\n";
    }

    {
        const auto isDeviceAvailable = device.getInfo<CL_DEVICE_AVAILABLE>();
        out << "Device available: " << toStringBool(isDeviceAvailable) << "\n";
    }

    {
        const auto compilerAvailable = device.getInfo<CL_DEVICE_COMPILER_AVAILABLE>();
        out << "Compiler available: " << toStringBool(compilerAvailable) << "\n";
    }

#if defined(CL_VERSION_1_2)
    {
        const auto linkerAvailable = device.getInfo<CL_DEVICE_LINKER_AVAILABLE>();
        out << "Linker available: " << toStringBool(linkerAvailable) << "\n";
    }
#endif

    // ...

#if defined(CL_VERSION_2_0)
    {
        const auto maxOnDeviceEvents = device.getInfo<CL_DEVICE_MAX_ON_DEVICE_EVENTS>();
        out << "Max number of events in use by a device queue: " << maxOnDeviceEvents << "\n";
    }
#endif

#if defined(CL_VERSION_1_2)
    {
        const auto builtinKernels = device.getInfo<CL_DEVICE_BUILT_IN_KERNELS>();
        out << "Built in kernels: " << builtinKernels << "\n";
    }
#endif

#if defined(CL_VERSION_3_0)
    {
        const auto builtinKernelsWithVersion =
          device.getInfo<CL_DEVICE_BUILT_IN_KERNELS_WITH_VERSION>();
        out << "Built in kernels with version:\n"
            << toStringNameVersions(builtinKernelsWithVersion);
    }
#endif

    {
        const auto platformId = device.getInfo<CL_DEVICE_PLATFORM>();
        out << "Platform Id: " << platformId << "\n";
    }

    {
        const auto deviceName = device.getInfo<CL_DEVICE_NAME>();
        out << "Device name: " << deviceName << "\n";
    }

    {
        const auto vendor = device.getInfo<CL_DEVICE_VENDOR>();
        out << "Vendor: " << vendor << "\n";
    }

    {
        const auto driverVersion = device.getInfo<CL_DRIVER_VERSION>();
        out << "Driver version: " << driverVersion << "\n";
    }

    {
        const auto deviceProfile = device.getInfo<CL_DEVICE_PROFILE>();
        out << "Device profile: " << deviceProfile << "\n";
    }

    {
        const auto deviceVersion = device.getInfo<CL_DEVICE_VERSION>();
        out << "Device version: " << deviceVersion << "\n";
    }

#if defined(CL_VERSION_3_0)
    {
        const auto deviceNumericVersion = device.getInfo<CL_DEVICE_NUMERIC_VERSION>();
        out << "Device numeric version: " << deviceNumericVersion << "\n";
    }
#endif

#if defined(CL_VERSION_1_1) && !defined(CL_VERSION_3_0)
    {
        const auto openclCVersion = device.getInfo<CL_DEVICE_OPENCL_C_VERSION>();
        out << "OpenCL C version: " << openclCVersion << "\n";
    }
#endif

#if defined(CL_VERSION_3_0)
    {
        const auto openclCAllVersions = device.getInfo<CL_DEVICE_OPENCL_C_ALL_VERSIONS>();
        out << "OpenCL C all versions:\n" << toStringNameVersions(openclCAllVersions);
    }
#endif

#if defined(CL_VERSION_3_0)
    {
        const auto openclCFeatures = device.getInfo<CL_DEVICE_OPENCL_C_FEATURES>();
        out << "OpenCL C features:\n" << toStringNameVersions(openclCFeatures);
    }
#endif

    {
        const auto extensions = device.getInfo<CL_DEVICE_EXTENSIONS>();
        out << "Device extensions: " << extensions;
    }

    // ...

#if defined(CL_VERSION_3_0)
    {
        const auto atomicFenceCapabilities = device.getInfo<CL_DEVICE_ATOMIC_FENCE_CAPABILITIES>();
        out << "Atomic fence capabilities: " << toStringAtomicCapabilities(atomicFenceCapabilities)
            << "\n";
    }
#endif

#if defined(CL_VERSION_3_0)
    {
        const auto pipeSupport = device.getInfo<CL_DEVICE_PIPE_SUPPORT>();
        out << "Pipes supported: " << toStringBool(pipeSupport) << "\n";
    }
#endif

#if defined(CL_VERSION_3_0)
    {
        const auto preferredWorkGroupSizeMultiple =
          device.getInfo<CL_DEVICE_PREFERRED_WORK_GROUP_SIZE_MULTIPLE>();
        out << "Preferred work group size multiple: " << preferredWorkGroupSizeMultiple << "\n";
    }
#endif

#if defined(CL_VERSION_3_0)
    {
        const auto latestConformanceVersionPassed =
          device.getInfo<CL_DEVICE_LATEST_CONFORMANCE_VERSION_PASSED>();
        out << "Latest conformance version passed: " << latestConformanceVersionPassed << "\n";
    }
#endif

#if defined(CL_VERSION_3_0)
    {
    }
#endif
}

void OpenclUtils::printPlatformsAndDevices(std::ostream& out)
{
    cl::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);

    for (const auto& platform : platforms)
    {
        out << "\n================================================================\n";
        out << "Platform:\n";
        printPlatform(platform, out);
        out << "\nDevices:\n";
        printDevices(platform, out);
    }
}

void OpenclUtils::printContextInfo(cl::Context context, std::ostream& out)
{
    out << "Context info:\n";
    out << "Pointer address: " << context() << "\n";

    {
        const auto referenceCount = context.getInfo<CL_CONTEXT_REFERENCE_COUNT>();
        out << "Reference count: " << referenceCount << "\n";
    }

    {
        const auto deviceCount = context.getInfo<CL_CONTEXT_NUM_DEVICES>();
        out << "Device count: " << deviceCount << "\n";
    }

    {
        const auto devices = context.getInfo<CL_CONTEXT_DEVICES>();
        if (!devices.empty())
        {
            out << "Devices Id:\n";
            for (const auto& device : devices)
            {
                out << device() << "\n";
            }
        }
    }

    {
        const auto properties = context.getInfo<CL_CONTEXT_PROPERTIES>();
        out << "Context properties:\n";
        for (size_t i = 0; i + 1 < properties.size(); i += 2)
        {
            const auto propertyName = properties[i];
            const auto propertyValue = properties[i + 1];
            out << toStringProperty(static_cast<cl_uint>(propertyName), propertyValue) << "\n";
        }
    }
}
}
