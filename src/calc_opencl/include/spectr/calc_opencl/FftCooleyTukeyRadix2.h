#pragma once

#include <spectr/calc_opencl/OpenclApi.h>

#include <complex>
#include <vector>

namespace spectr::calc_opencl
{
class FftCooleyTukeyRadix2
{
public:
    FftCooleyTukeyRadix2(cl::Context context, size_t fftSize);

    ~FftCooleyTukeyRadix2();

    /**
     * @brief Executes FFT on GPU, then returns.
     * @param realValues Array of real values of function f(x).
     */
    void execute(const std::vector<float>& functionValues);

    /**
     * @brief Get GPU OpenCL buffer with FFT complex values. Must be called after execute(). //TODO?
     * @return OpenCL buffer.
     */
    cl::Buffer getFftBufferGpu();

    /**
     * @brief Get CPU buffer with FFT complex values. Must be called after execute().
     * @return
     */
    std::vector<std::complex<float>> getFffBufferCpu();

    /**
     * @brief Copies magnitude values of FFT frequencies to OpenGL buffer.
     * @param openglBuffer Destination OpenGL buffer.
     * @param elementOffset Buffer offset in elements (element = real number).
     */
    void copyMagnitudesTo(cl::BufferGL openglBuffer, cl_uint elementOffset, float* maxMagnitude = nullptr);

    cl::Context getContext() const;

private:
    cl::Context m_context;
    cl::Device m_device;
    cl::Program m_program;
    cl::Buffer m_buffer1;
    cl::Buffer m_buffer2;
    cl::Buffer m_magnitudesBuffer;
    cl::Buffer m_maxValueBuffer;
    std::vector<cl::Buffer> m_omegaBuffers;
    cl::CommandQueue m_queue;
    const size_t m_fftSize;
    const size_t m_stageCount;
};
}
