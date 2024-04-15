#pragma once

#include <spectr/calc_opencl/OpenclApi.h>
#include <spectr/calc_opencl/OpenclUtils.h>

#include <cstdint>

namespace spectr::calc_opencl
{
class RtsaUpdater
{
public:
    RtsaUpdater(cl::Context context,
                size_t frequencyCount,
                size_t magnitudeResolution,
                size_t historyBuffersCount,
                float magnitudeDbfsRange,
                size_t bufferSize);

    ~RtsaUpdater();

    void update(cl::Buffer magnitudesBuffer, uint32_t openglBuffer, float referenceValue);

private:
    const size_t m_frequencyCount;
    const size_t m_magnitudeResolution;
    const size_t m_historyBuffersCount;
    const float m_magnitudeDbfsRange;
    cl::Context m_context;
    cl::Device m_device;
    cl::Program m_program;
    cl::CommandQueue m_queue;
    cl::Buffer m_historyBuffer;
    size_t m_nextBufferIndex = 0;
    size_t m_bufferSize;

    // Used to speed up copy operations
    cl::Buffer m_workBuffer;
    float* m_hostPinnedMemory;
    float* m_hostMappedMemory;
    uint32_t m_openglBuffer;
};
}
