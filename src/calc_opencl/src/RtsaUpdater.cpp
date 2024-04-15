#include <spectr/calc_opencl/RtsaUpdater.h>

#include <spectr/render_gl/GraphicsApi.h>

#include <spectr/utils/Asset.h>
#include <spectr/utils/Exception.h>
#include <spectr/utils/File.h>

namespace spectr::calc_opencl
{
namespace
{
const std::string KernelAssetPath{ "opencl/DensityHeatmapUpdater.cl" };
}

RtsaUpdater::RtsaUpdater(cl::Context context,
                         size_t frequencyCount,
                         size_t magnitudeResolution,
                         size_t historyBuffersCount,
                         float magnitudeDbfsRange,
                         size_t bufferSize)
  : m_frequencyCount{ frequencyCount }
  , m_magnitudeResolution{ magnitudeResolution }
  , m_historyBuffersCount{ historyBuffersCount }
  , m_magnitudeDbfsRange{ magnitudeDbfsRange }
  , m_context{ context }
  , m_device{ OpenclUtils::getDevice(m_context) }
  , m_queue{ m_context }
  , m_bufferSize{ bufferSize }
  , m_hostMappedMemory{ NULL }
{
    // allocate history buffer
    const auto historyBufferSize = sizeof(float) * m_frequencyCount * m_historyBuffersCount;
    m_historyBuffer = cl::Buffer(m_context, CL_MEM_READ_WRITE, historyBufferSize);

    m_queue.enqueueFillBuffer<cl_float>(m_historyBuffer, -1000.0f, 0, historyBufferSize);

    // compile OpenCL program
    const auto sourcePath = utils::Asset::getPath(KernelAssetPath);
    const auto source = utils::File::read(sourcePath);
    m_program = { m_context, source };
    try
    {
        std::stringstream ss;
        ss << "-cl-std=CL2.0";
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

    // Allocate work buffer
    m_workBuffer = cl::Buffer(m_context, CL_MEM_READ_WRITE, m_bufferSize);
    m_hostPinnedMemory = (float*)m_queue.enqueueMapBuffer(m_workBuffer, false, 0, 0, m_bufferSize);
}

RtsaUpdater::~RtsaUpdater() {
    m_queue.enqueueUnmapMemObject(m_workBuffer, m_hostPinnedMemory);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_openglBuffer);
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void RtsaUpdater::update(cl::Buffer magnitudesBuffer,
                         uint32_t openglBuffer,
                         float referenceValue)
{
    // calculate dBFS values
    {
        /*OpenclUtils::printVector<float>(
          m_queue, magnitudesBuffer, m_frequencyCount, "Magnitudes values:\n");*/

        // TODO copy to other buffer instead of calculating in place ?

        auto calculateDbfsKernel = cl::KernelFunctor<cl::Buffer, float>(m_program, "convertToDBFS");

        const cl::NDRange globalGroupSize{ m_frequencyCount };
        const cl::NDRange localGroupSize{ std::min(m_frequencyCount, static_cast<size_t>(64)) };
        const cl::EnqueueArgs enqueueArgs(m_queue, globalGroupSize, localGroupSize);
        calculateDbfsKernel(enqueueArgs, magnitudesBuffer, referenceValue);

        // OpenclUtils::printVector<float>(m_queue, magnitudesBuffer, m_frequencyCount, "");
    }

    // copy to the storage buffer
    const auto currentHistoryBuffer = m_nextBufferIndex;
    m_nextBufferIndex = (m_nextBufferIndex + 1) % m_historyBuffersCount;
    {
        const auto magnitudeBufferSize = sizeof(float) * m_frequencyCount;
        const auto dstOffset = magnitudeBufferSize * currentHistoryBuffer;

        m_queue.enqueueCopyBuffer(
          magnitudesBuffer, m_historyBuffer, 0, dstOffset, magnitudeBufferSize);
    }

    if (!m_hostMappedMemory) {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, openglBuffer);
        m_hostMappedMemory = (float*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        m_openglBuffer = openglBuffer;
    }

    // update the density heatmap
    {
        auto updateDensityHeatmapKernel =
          cl::KernelFunctor<cl::Buffer, cl::Buffer, cl_uint, cl_uint, cl_uint, cl_uint, float>(
            m_program, "updateDensityHeatmap");
        
        const cl::NDRange globalGroupSize{ m_frequencyCount, m_magnitudeResolution };
        const cl::EnqueueArgs enqueueArgs(m_queue, globalGroupSize);
        
        const auto magnitudeIndexToDbfsCoeff = m_magnitudeDbfsRange / m_magnitudeResolution;
                
        updateDensityHeatmapKernel(enqueueArgs,
                                   m_workBuffer,
                                   m_historyBuffer,
                                   static_cast<cl_uint>(m_frequencyCount),
                                   static_cast<cl_uint>(m_magnitudeResolution),
                                   static_cast<cl_uint>(m_historyBuffersCount),
                                   static_cast<cl_uint>(currentHistoryBuffer),
                                   static_cast<float>(magnitudeIndexToDbfsCoeff));

        m_queue.enqueueReadBuffer(m_workBuffer, false, 0, m_bufferSize, m_hostMappedMemory);
        m_queue.finish();

        //std::memcpy(m_hostMappedMemory, m_hostPinnedMemory, m_bufferSize);
    }
}
}
