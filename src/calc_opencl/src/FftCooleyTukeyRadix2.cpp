#include <spectr/calc_opencl/FftCooleyTukeyRadix2.h>

#include <spectr/utils/Asset.h>
#include <spectr/utils/Exception.h>
#include <spectr/utils/File.h>

#include <spdlog/spdlog.h>

#include <cmath>
#include <complex>
#include <sstream>
#include <string>

namespace spectr::calc_opencl
{
using Complex = std::complex<float>;
static_assert(sizeof(Complex) == 2 * sizeof(cl_float));

constexpr auto MaxFftPowerOfTwo = 18;

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

    /*std::vector<float> outputRealValues;
    std::transform(complexValues.begin(),
                   complexValues.end(),
                   std::back_inserter(outputRealValues),
                   [](const Complex& v) { return v.real(); });*/
}
}

FftCooleyTukeyRadix2::FftCooleyTukeyRadix2(cl::Context context, size_t fftSize)
  : m_context{ context }
  , m_fftSize{ fftSize }
  , m_stageCount{ getPowerOfTwo(m_fftSize) }
{
    if (m_stageCount > MaxFftPowerOfTwo)
    {
        throw utils::Exception("FFT size is too big. Max FFT size supported is 2^{}",
                               MaxFftPowerOfTwo);
    }

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

    // TODO log(m_program);

    // allocate two work buffers
    const auto complexNumberSize = 2 * sizeof(cl_float);
    const auto valuesBufferByteCount = m_fftSize * complexNumberSize;

    m_buffer1 = { m_context, CL_MEM_READ_WRITE, valuesBufferByteCount };
    m_buffer2 = { m_context, CL_MEM_READ_WRITE, valuesBufferByteCount };
    m_queue = cl::CommandQueue{ m_context };
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
      cl::EnqueueArgs(m_queue, cl::NDRange(m_fftSize), cl::NDRange(m_fftSize)),
      m_buffer1,
      m_buffer2);

    auto fftStageKernel =
      cl::KernelFunctor<cl::Buffer, cl::Buffer, cl_uint, cl_uint, cl_uint>(m_program, "fft_stage");

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

        const cl::NDRange globalGroupSize{ subFftCount, subFftHalfSize };

        const cl::NDRange localGroupSize{ std::min(subFftCount, 64ull),
                                          std::min(subFftHalfSize, 64ull) };

        const cl::EnqueueArgs enqueueArgs(m_queue, globalGroupSize, localGroupSize);
        fftStageKernel(enqueueArgs, srcBuffer, dstBuffer, subFftSize, subFftCount, stageIndex);

        /* spdlog::debug("After stage {}:", stageIndex);
         printComplexNumbers(queue, buffer1, m_fftSize);
         printComplexNumbers(queue, buffer2, m_fftSize);
         spdlog::debug("-----------------------------------------------------", stageIndex);*/

        // TODO swap buffers
        // std::swap(buffer1, buffer2);
    }

    m_queue.finish();
}

cl::Buffer FftCooleyTukeyRadix2::getFinalDataBufferGpu()
{
    const auto finalOutputBuffer = (m_stageCount % 2 == 0) ? m_buffer2 : m_buffer1;
    return finalOutputBuffer;
}

std::vector<float> FftCooleyTukeyRadix2::getFinalDataBufferCpu()
{
    // copy the result to host
    std::vector<Complex> outputComplexValues;
    outputComplexValues.resize(m_fftSize);
    cl::copy(
      m_queue, getFinalDataBufferGpu(), outputComplexValues.begin(), outputComplexValues.end());

    std::vector<float> outputRealValues;
    std::transform(outputComplexValues.begin(),
                   outputComplexValues.end(),
                   std::back_inserter(outputRealValues),
                   [](const Complex& v) { return v.real(); });

    return outputRealValues;
}

void FftCooleyTukeyRadix2::copyFrequenciesTo(cl::Buffer gpuBuffer, cl_uint elementOffset)
{
    auto convertComplexToRealKernel =
      cl::KernelFunctor<cl::Buffer, cl::Buffer, size_t>(m_program, "convert_fft_to_frequencies");
    const auto copiesCount = m_fftSize / 2;
    const cl::NDRange globalGroupSize{ copiesCount };
    const cl::NDRange localGroupSize{ std::min(copiesCount, 64ull) };
    const cl::EnqueueArgs enqueueArgs(m_queue, globalGroupSize, localGroupSize);
    convertComplexToRealKernel(enqueueArgs, getFinalDataBufferGpu(), gpuBuffer, elementOffset);
}

cl::Context FftCooleyTukeyRadix2::getContext() const
{
    return m_context;
}
}
