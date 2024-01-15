#define _USE_MATH_DEFINES

// Suppress Visual Studio warnings for iterators
#define _SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING
#define _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS

#include <spectr/calc_opencl/FftCooleyTukeyRadix2.h>

#include <spectr/calc_cpu/FftCooleyTukeyUtils.h>

#include <spectr/utils/Asset.h>
#include <spectr/utils/Exception.h>
#include <spectr/utils/File.h>
#include <spectr/utils/Timer.h>

#include <spdlog/spdlog.h>

#include <algorithm>
#include <cmath>
#include <complex>
#include <sstream>
#include <string>

namespace spectr::calc_opencl
{
using Complex = std::complex<float>;
static_assert(sizeof(Complex) == 2 * sizeof(cl_float));

namespace
{
const std::string KernelAssetPath = "opencl/FFTCooleyTukeyRadix2Float.cl";

bool isPowerOfTwo(size_t number, size_t& power)
{
    power = 0;
    while (number % 2 == 0)
    {
        power++;
        number /= 2;
    }
    return number == 1;
}

size_t getPowerOfTwo(size_t number)
{
    size_t powerOfTwo;
    if (!isPowerOfTwo(number, powerOfTwo))
    {
        throw utils::Exception("FFT size parameter must be a power of two, but is: {}", number);
    }
    return powerOfTwo;
}

void printComplexNumbers(cl::CommandQueue commandQueue,
                         cl::Buffer buffer,
                         size_t complexNumbersCount)
{
    std::vector<Complex> complexValues;
    complexValues.resize(complexNumbersCount);
    cl::copy(commandQueue, buffer, complexValues.begin(), complexValues.end());

    std::stringstream ss;
    ss << "\nComplex numbers count: " << complexNumbersCount << "\n";
    for (const auto& complexNumber : complexValues)
    {
        ss << complexNumber << "\n";
    }

    spdlog::debug(ss.str());
}

template<typename T>
void print(cl::CommandQueue commandQueue,
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

    spdlog::debug(ss.str());
}

cl::Device getDevice(cl::Context context)
{
    const auto devices = context.getInfo<CL_CONTEXT_DEVICES>();
    ASSERT(devices.size() == 1);
    return devices[0];
}
}

FftCooleyTukeyRadix2::FftCooleyTukeyRadix2(cl::Context context, size_t fftSize)
  : m_context{ context }
  , m_device{ getDevice(m_context) }
  , m_fftSize{ fftSize }
  , m_stageCount{ getPowerOfTwo(m_fftSize) }
{
    /* if (m_stageCount > MaxFftPowerOfTwo)
     {
         throw utils::Exception("FFT size is too big. Max FFT size supported is 2^{}",
                                MaxFftPowerOfTwo);
     }*/

    const auto sourcePath = utils::Asset::getPath(KernelAssetPath);
    const auto source = utils::File::read(sourcePath);

    m_program = { m_context, source };
    try
    {
        const auto bitReverseShiftValue = (CHAR_BIT * sizeof(uint32_t)) - m_stageCount;

        std::stringstream ss;
        ss << "-cl-std=CL2.0";
        ss << " -DBIT_REVERSE_SHIFT_VALUE=" << bitReverseShiftValue;
        ss << " -DFFT_SIZE=" << m_fftSize;
        const auto compilerDirectives = ss.str();
        m_program.build(compilerDirectives.c_str());
    }
    catch (const cl::BuildError& ex)
    {
        const auto buildOptions = m_program.getBuildInfo<CL_PROGRAM_BUILD_OPTIONS>();
        const auto buildStatus = m_program.getBuildInfo<CL_PROGRAM_BUILD_STATUS>();
        const auto buildInfo = m_program.getBuildInfo<CL_PROGRAM_BUILD_LOG>();

        std::stringstream ss;
        for (const auto& pair : ex.getBuildLog())
        {
            ss << pair.second << "\n";
        }

        throw utils::Exception(
          "Failed to build a kernel. Error code: {}\n Error log:\n{}", ex.err(), ss.str());
    }

    // allocate two work buffers
    const auto complexNumberSize = 2 * sizeof(cl_float);
    const auto valuesBufferByteCount = m_fftSize * complexNumberSize;

    m_buffer1 = { m_context, CL_MEM_READ_WRITE, valuesBufferByteCount };
    m_buffer2 = { m_context, CL_MEM_READ_WRITE, valuesBufferByteCount };
    m_magnitudesBuffer = { m_context, CL_MEM_READ_WRITE, valuesBufferByteCount / 2 };
    m_maxValueBuffer = { m_context, CL_MEM_READ_WRITE, valuesBufferByteCount / 2 };
    m_queue = cl::CommandQueue{ m_context };

    // pre-calculate omega buffers
    for (size_t stageIndex = 0; stageIndex < m_stageCount; ++stageIndex)
    {
        const auto subFftHalfSize = 1 << stageIndex;
        const auto omegas = calc_cpu::FftCooleyTukeyUtils<float>::getOmegas(stageIndex);
        cl::Buffer omegaBuffer{ m_context, omegas.begin(), omegas.end(), true };
        m_omegaBuffers.push_back(omegaBuffer);
    }
}

FftCooleyTukeyRadix2::~FftCooleyTukeyRadix2()
{
    //
}

void FftCooleyTukeyRadix2::execute(const std::vector<float>& realValues)
{
    if (realValues.size() != m_fftSize)
    {
        throw utils::Exception(
          "Actual values count doesn't equal to the expected FFT elements count. Expected: {}",
          m_fftSize);
    }

    // copy data to the first buffer
    std::vector<Complex> complexValues(realValues.begin(), realValues.end());
    cl::copy(m_queue, complexValues.begin(), complexValues.end(), m_buffer1);

    // perform bit-reverse permutation
    auto bitReversePermutationKernel =
      cl::KernelFunctor<cl::Buffer, cl::Buffer>(m_program, "bit_reverse_permutation");

    bitReversePermutationKernel(
      cl::EnqueueArgs(
        m_queue, cl::NDRange(m_fftSize), cl::NDRange(std::min(m_fftSize, static_cast<size_t>(64)))),
      m_buffer1,
      m_buffer2);

    auto fftStageKernel =
      cl::KernelFunctor<cl::Buffer, cl::Buffer, cl::Buffer, cl_uint, cl_uint, cl_uint>(m_program,
                                                                                       "fft_stage");

    for (size_t stageIndex = 0; stageIndex < m_stageCount; ++stageIndex)
    {
        /*spdlog::debug("Before stage {}:", stageIndex);
        printComplexNumbers(queue, buffer1, m_fftSize);
        printComplexNumbers(queue, buffer2, m_fftSize);*/

        const auto isEvenStage = (stageIndex % 2 == 0);
        const auto srcBuffer = isEvenStage ? m_buffer2 : m_buffer1;
        const auto dstBuffer = isEvenStage ? m_buffer1 : m_buffer2;

        const auto subFftSize = 1ull << (stageIndex + 1ull);
        const auto subFftHalfSize = subFftSize / 2;
        const auto subFftCount = m_fftSize / subFftSize;

        const auto omegaBuffer = m_omegaBuffers[stageIndex];

        const cl::NDRange globalGroupSize{ subFftCount, subFftHalfSize };

        const cl::NDRange localGroupSize{ std::min(subFftCount, 64ull),
                                          std::min(subFftHalfSize, 64ull) };

        const cl::EnqueueArgs enqueueArgs(m_queue, globalGroupSize); //, localGroupSize); // TODO
        fftStageKernel(enqueueArgs,
                       srcBuffer,
                       dstBuffer,
                       omegaBuffer,
                       static_cast<cl_uint>(subFftSize),
                       static_cast<cl_uint>(subFftCount),
                       static_cast<cl_uint>(stageIndex));

        /* spdlog::debug("After stage {}:", stageIndex);
         printComplexNumbers(queue, buffer1, m_fftSize);
         printComplexNumbers(queue, buffer2, m_fftSize);
         spdlog::debug("-----------------------------------------------------", stageIndex);*/

        // TODO swap buffers
        // std::swap(buffer1, buffer2);
    }

    m_queue.finish();
}

cl::Buffer FftCooleyTukeyRadix2::getFftBufferGpu()
{
    const auto finalOutputBuffer = (m_stageCount % 2 == 0) ? m_buffer2 : m_buffer1;
    return finalOutputBuffer;
}

std::vector<std::complex<float>> FftCooleyTukeyRadix2::getFffBufferCpu()
{
    std::vector<Complex> values;
    values.resize(m_fftSize);
    cl::copy(m_queue, getFftBufferGpu(), values.begin(), values.end());
    return values;
}

void FftCooleyTukeyRadix2::copyMagnitudesTo(cl::BufferGL openglBuffer,
                                            cl_uint elementOffset,
                                            float* maxMagnitude)
{
    const auto valuesCount = m_fftSize / 2;

    // calculate magnitudes
    {
        auto calculateMagnitudesKernel =
          cl::KernelFunctor<cl::Buffer, cl::Buffer>(m_program, "calculate_magnitudes");
        const cl::NDRange globalGroupSize{ valuesCount };
        const cl::NDRange localGroupSize{ std::min(valuesCount, static_cast<size_t>(64)) };
        const cl::EnqueueArgs enqueueArgs(m_queue, globalGroupSize, localGroupSize);
        calculateMagnitudesKernel(enqueueArgs, getFftBufferGpu(), m_magnitudesBuffer);
    }

    // find max magnitude
    if (maxMagnitude)
    {
        auto findMaxKernel = cl::Kernel(m_program, "find_max");
        const auto workGroupSize =
          findMaxKernel.getWorkGroupInfo<CL_KERNEL_WORK_GROUP_SIZE>(m_device);
        const cl::NDRange localSize{ workGroupSize };
        const cl::NDRange globalSize{ valuesCount };
        const cl::EnqueueArgs enqueueArgs{ m_queue, globalSize, localSize };
        findMaxKernel.setArg(0, m_magnitudesBuffer);
        findMaxKernel.setArg(1, sizeof(float) * workGroupSize, nullptr);
        findMaxKernel.setArg(2, m_maxValueBuffer);
        m_queue.enqueueNDRangeKernel(findMaxKernel, cl::NullRange, globalSize, localSize);

        const auto reducedMagnitudesCount = valuesCount / workGroupSize;
        std::vector<float> values;
        values.resize(reducedMagnitudesCount);
        cl::copy(m_queue, m_maxValueBuffer, values.begin(), values.end());

        *maxMagnitude = *std::max_element(values.begin(), values.end());
    }

    // copy magnitudes to the OpenGL buffer
    {
        const cl::vector<cl::Memory> memoryObjects{ cl::Memory(openglBuffer.get(), true) };

        // utils::Timer testTimer;
        m_queue.enqueueAcquireGLObjects(&memoryObjects);
        // spdlog::trace("enqueueAcquireGLObjects() duration: {}", testTimer.toString());

        m_queue.enqueueCopyBuffer(m_magnitudesBuffer,
                                  openglBuffer,
                                  0,
                                  sizeof(float) * elementOffset,
                                  sizeof(float) * valuesCount);

        m_queue.enqueueReleaseGLObjects(&memoryObjects);
    }
}

cl::Context FftCooleyTukeyRadix2::getContext() const
{
    return m_context;
}
}
