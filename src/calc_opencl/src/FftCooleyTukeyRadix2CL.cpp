#include <spectr/calc_opencl/FftCooleyTukeyRadix2CL.h>

#include <spectr/calc_cpu/FftCooleyTukeyUtils.h>
#include <spectr/calc_opencl/OpenclUtils.h>
#include <spectr/utils/Asset.h>
#include <spectr/utils/Exception.h>
#include <spectr/utils/File.h>
#include <spectr/utils/Timer.h>

#include <spdlog/spdlog.h>

#include <algorithm>
#include <cmath>
#include <sstream>
#include <string>

namespace spectr::calc_opencl
{
namespace
{
const std::string ProgramAssetPath = "opencl/FFTCooleyTukeyRadix2Float.cl";
}

FftCooleyTukeyRadix2::FftCooleyTukeyRadix2(cl::Context context, size_t fftSize)
  : m_fftSize{ fftSize }
  , m_stageCount{ utils::Math::getPowerOfTwo(m_fftSize) }
  , m_context{ context }
  , m_device{ OpenclUtils::getDevice(m_context) }
  , m_queue{ m_context }
{
    const auto sourcePath = utils::Asset::getPath(ProgramAssetPath);
    const auto source = utils::File::read(sourcePath);
    m_program = cl::Program{ m_context, source };
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

    m_workBuffers[0] = { m_context, CL_MEM_READ_WRITE, valuesBufferByteCount };
    m_workBuffers[1] = { m_context, CL_MEM_READ_WRITE, valuesBufferByteCount };
    m_magnitudesBuffer = { m_context, CL_MEM_READ_WRITE, valuesBufferByteCount / 2 };
    m_maxValueBuffer = { m_context, CL_MEM_READ_WRITE, valuesBufferByteCount / 2 };

    // pre-calculate omega buffers
    m_omegaBuffers.reserve(m_stageCount);
    for (size_t stageIndex = 0; stageIndex < m_stageCount; ++stageIndex)
    {
        const auto subFftHalfSize = 1 << stageIndex;
        const auto omegas = calc_cpu::FftCooleyTukeyUtils::getOmegas<float>(stageIndex);
        cl::Buffer omegaBuffer{ m_context, omegas.begin(), omegas.end(), true };
        m_omegaBuffers.push_back(std::move(omegaBuffer));
    }
}

cl::Context FftCooleyTukeyRadix2::getContext() const
{
    return m_context;
}

void FftCooleyTukeyRadix2::execute(const std::vector<float>& realValues)
{
    if (realValues.size() != m_fftSize)
    {
        throw utils::Exception(
          "Actual values count doesn't equal to the expected FFT elements count. Expected: {}",
          m_fftSize);
    }

    // copy the signal data to the first buffer
    std::vector<Complex> complexValues(realValues.begin(), realValues.end());
    complexValues[0] = { 1 };
    complexValues[1] = { 2 };

    // TODO non-blocking copy?
    cl::copy(m_queue, complexValues.begin(), complexValues.end(), m_workBuffers[0]);

    // perform bit-reverse permutation
    auto bitReversePermutationKernel =
      cl::KernelFunctor<cl::Buffer, cl::Buffer>(m_program, "bit_reverse_permutation");

    const cl::EnqueueArgs enqueueArgs(m_queue, cl::NDRange(m_fftSize));
    bitReversePermutationKernel(enqueueArgs, m_workBuffers[0], m_workBuffers[1]);
    std::swap(m_workBuffers[0], m_workBuffers[1]);

    auto fftStageKernel =
      cl::KernelFunctor<cl::Buffer, cl::Buffer, cl::Buffer, cl_uint, cl_uint, cl_uint>(m_program,
                                                                                       "fft_stage");

    for (size_t stageIndex = 0; stageIndex < m_stageCount; ++stageIndex)
    {
        // spdlog::debug("Before FFT stage {}:", stageIndex);
        // OpenclUtils::printComplexNumbers(m_queue, m_workBuffers[0], m_fftSize);
        // OpenclUtils::printComplexNumbers(m_queue, m_workBuffers[1], m_fftSize);

        const auto& srcBuffer = m_workBuffers[0];
        const auto& dstBuffer = m_workBuffers[1];

        const auto subFftSize = 1ull << (stageIndex + 1ull);
        const auto subFftHalfSize = subFftSize / 2;
        const auto subFftCount = m_fftSize / subFftSize;

        const auto omegaBuffer = m_omegaBuffers[stageIndex];

        const cl::NDRange globalGroupSize{ subFftCount, subFftHalfSize };

        /*const cl::NDRange localGroupSize{ std::min(subFftCount, 64ull),
                                          std::min(subFftHalfSize, 64ull) };*/

        const cl::EnqueueArgs enqueueArgs(m_queue, globalGroupSize); //, localGroupSize);
        fftStageKernel(enqueueArgs,
                       srcBuffer,
                       dstBuffer,
                       omegaBuffer,
                       static_cast<cl_uint>(subFftSize),
                       static_cast<cl_uint>(subFftCount),
                       static_cast<cl_uint>(stageIndex));

        // spdlog::debug("After FFT stage {}:", stageIndex);
        // OpenclUtils::printComplexNumbers(m_queue, m_workBuffers[0], m_fftSize);
        // OpenclUtils::printComplexNumbers(m_queue, m_workBuffers[1], m_fftSize);
        // spdlog::debug("------------------------------------------------------", stageIndex);

        std::swap(m_workBuffers[0], m_workBuffers[1]);
    }

    m_queue.finish();
}

cl::Buffer FftCooleyTukeyRadix2::getFftBufferGpu()
{
    return m_workBuffers[0];
}

std::vector<std::complex<float>> FftCooleyTukeyRadix2::getFffBufferCpu()
{
    std::vector<Complex> values;
    values.resize(m_fftSize);
    cl::copy(m_queue, getFftBufferGpu(), values.begin(), values.end());
    return values;
}

void FftCooleyTukeyRadix2::calculateMagnitudes()
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
        // OpenclUtils::printVector<float>(m_queue, m_magnitudesBuffer, valuesCount, "Magnitudes:");
    }
}

void FftCooleyTukeyRadix2::copyMagnitudesTo(cl::BufferGL openglBuffer,
                                            cl_uint elementOffset,
                                            float* maxMagnitude)
{
    const auto valuesCount = m_fftSize / 2;

    // find max magnitude // TODO disable or calculate mathematically???
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

        cl::Event finishEvent;
        m_queue.enqueueReleaseGLObjects(&memoryObjects, nullptr, &finishEvent);
        finishEvent.wait();
    }
}

cl::Buffer FftCooleyTukeyRadix2::getMagnitudesBuffer()
{
    return m_magnitudesBuffer;
}
}
