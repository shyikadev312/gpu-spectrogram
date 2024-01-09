#pragma once

#include <spectr/calc_opencl/OpenclApi.h>

namespace spectr::calc_opencl
{
class FftCooleyTukeyRadix2
{
public:
    FftCooleyTukeyRadix2(cl::Context context, size_t fftSize);

    ~FftCooleyTukeyRadix2();

    void execute(const std::vector<float>& realValues);

    cl::Buffer getFinalDataBufferGpu();

    std::vector<float> getFinalDataBufferCpu();

    void copyFrequenciesTo(cl::Buffer gpuBuffer, cl_uint elementOffset);

    cl::Context getContext() const;

private:
    cl::Context m_context{};
    cl::Program m_program{};
    cl::Buffer m_buffer1{};
    cl::Buffer m_buffer2{};
    cl::CommandQueue m_queue{};
    const size_t m_fftSize;
    const size_t m_stageCount;
};
}
