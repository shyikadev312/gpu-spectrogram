#include <spectr/desktop_app/AudioFileTimeFrequencyWorker.h>

#include <spectr/calc_cpu/FftCooleyTukeyRadix2.h>
#include <spectr/calc_cpu/FftCooleyTukeyUtils.h>
#include <spectr/calc_cuda/FftCooleyTukeyRadix2Cuda.h>
#include <spectr/utils/Math.h>
#include <spectr/utils/Timer.h>
#include <spectr/utils/Assert.h>

#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>

namespace spectr::desktop_app
{
namespace
{
std::pair<float, float> minMax(const std::vector<float>& values)
{
    const auto minEl = std::min_element(values.begin(), values.end());
    const auto maxEl = std::max_element(values.begin(), values.end());
    return { *minEl, *maxEl };
}
}

AudioFileTimeFrequencyWorker::AudioFileTimeFrequencyWorker(
  AudioFileTimeFrequencyWorkerSettings settings)
  : m_settings { std::move(settings) }, m_rtsaGlBuffer(m_settings.rtsaHeatmapContainer->getBuffer()), bufferSize(settings.rtsaBufferSize)
  // , m_rtsaGlBuffer{ m_settings.fftCalculator->getContext(),
  //                   CL_MEM_READ_WRITE,
  //                   m_settings.rtsaHeatmapContainer->getBuffer() }
{
}

AudioFileTimeFrequencyWorker::~AudioFileTimeFrequencyWorker()
{
    if (m_workerThread)
    {
        ASSERT(m_workerThread->request_stop());
        m_workerThread->join();
    }
}

void AudioFileTimeFrequencyWorker::update()
{
    // get buffer
    auto buffer = m_settings.heatmapContainer->getOrAllocateBuffer(0);

    utils::Timer timer;
    PendingData calculationInputData;

    // calculate fft
    for (size_t i = 0; i < m_settings.fftCalculationsInSecond / 3 + 1; ++i)
    {
        utils::Timer globalFftTimer;

        // stage: get input data
        {
            std::lock_guard<std::mutex> lock{ m_mutex };
            if (m_pendingDatas.empty())
            {
                break;
            }

            calculationInputData = std::move(m_pendingDatas.front());
            m_pendingDatas.pop();
        }

        // stage: calculate FFT
        timer.restart();

        // OpenCL
        //m_settings.fftCalculator->execute(calculationInputData.values);
        // CUDA
        size_t stageCount = utils::Math::getPowerOfTwo(m_settings.fftSize);

        float* buffer0    = new float[m_settings.audioData.getSampleRate() / m_settings.fftCalculationsInSecond * 2];
        float* buffer1    = new float[m_settings.audioData.getSampleRate() / m_settings.fftCalculationsInSecond * 2];
        float* magnitudes = new float[m_settings.audioData.getSampleRate() / m_settings.fftCalculationsInSecond];
        std::vector<std::complex<float>>* omegas = new std::vector<std::complex<float>>[stageCount];

        constexpr size_t block_size = 64;

        for (size_t stageIndex = 0; stageIndex < stageCount; ++stageIndex)
        {
            const auto subFftHalfSize = 1 << stageIndex;
            omegas[stageIndex] = calc_cpu::FftCooleyTukeyUtils::getOmegas<float>(stageIndex);
        }

        std::memcpy(buffer0, calculationInputData.values, m_settings.audioData.getSampleRate() / m_settings.fftCalculationsInSecond * sizeof(float));

        bit_reverse_permutation_wrapper(buffer0, buffer1, m_settings.audioData.getSampleRate() / m_settings.fftCalculationsInSecond, block_size);

        std::swap(buffer0, buffer1);

        const auto complexNumberSize = 2 * sizeof(float);

        for (size_t stageIndex = 0; stageIndex < stageCount; ++stageIndex)
        {
            const auto& srcBuffer = buffer0;
            const auto& dstBuffer = buffer1;

            const auto subFftSize = 1ull << (stageIndex + 1ull);
            const auto subFftCount = m_settings.fftSize / subFftSize;

            const auto omegaBuffer = omegas[stageIndex];

            fft_stage_wrapper(srcBuffer,
                              dstBuffer,
                              reinterpret_cast<const float*>(omegaBuffer.data()),
                              static_cast<unsigned int>(subFftSize),
                              static_cast<unsigned int>(subFftCount),
                              static_cast<unsigned int>(stageIndex),
                              m_settings.audioData.getSampleRate() / m_settings.fftCalculationsInSecond,
                              omegaBuffer.size());

            std::swap(buffer0, buffer1);
        }

        // spdlog::trace("FFT calculation, size: {}, time: {}",
        //               calculationInputData.values.size(),
        //               timer.toString());

        std::cout << "FFT calculation, size: " << m_settings.oneFftSampleCount << ", time: " << timer.toString() << std::endl;

        // OpenCL
        // const auto fftResultGpu = m_settings.fftCalculator->getFffBufferCpu();
        // const auto magnitudesGPU =
        //   calc_cpu::FftCooleyTukeyRadix2::getMagnitudesFromFFT(fftResultGpu);

        //// debug stage: calculate on CPU, find max magnitude value
        // timer.restart();
        // const auto gpuMinMax = minMax(magnitudesGPU);
        // const auto maxMagnitudeLocal = gpuMinMax.second;
        // spdlog::trace("Magnitudes CPU calculation and finding max, time: {}", toString(timer));

        // spdlog::trace("Max magnitude found: {}", toString(timer));

        // constexpr bool CompareWithCpu = false;
        // if (CompareWithCpu)
        //{
        //     const auto fftResultCpu =
        //       calc_cpu::FftCooleyTukeyRadix2::getFFT(calculationInputData.values);
        //     const auto magnitudesCPU =
        //       calc_cpu::FftCooleyTukeyRadix2::getMagnitudesFromFFT(fftResultCpu);
        //     const auto cpuMinMax = minMax(magnitudesCPU);
        //     int x = 5;
        // }

        // stage: calculate and copy magnitudes to output buffer
        timer.restart();

        auto& heatmapBuffer =
          m_settings.heatmapContainer->getOrAllocateBuffer(calculationInputData.columnIndex);

        auto bufferIt = m_glBuffers.find(heatmapBuffer.startColumn);

        GLuint openglOpenclBuffer;
        if (bufferIt == m_glBuffers.end())
        {
            openglOpenclBuffer = heatmapBuffer.ssbo;
            m_glBuffers.insert({ heatmapBuffer.startColumn, openglOpenclBuffer });
        }
        else
        {
            openglOpenclBuffer = bufferIt->second;
        }

        const auto columnLocalIndex = calculationInputData.columnIndex - heatmapBuffer.startColumn;

        const auto elementOffsetInBuffer =
          columnLocalIndex * m_settings.heatmapContainer->getSettings().columnHeightElementCount;

        // stage: calculate magnitudes
        timer.restart();
        // OpenCL
        // m_settings.fftCalculator->calculateMagnitudes();
        // spdlog::trace("Magnitudes calculated: {}", timer.toString());
        // CUDA
        calculate_magnitudes_wrapper(buffer0, magnitudes, m_settings.audioData.getSampleRate() / m_settings.fftCalculationsInSecond, block_size);

        float maxMagnitudeLocal = *std::ranges::max(magnitudes, &magnitudes[m_settings.fftCalculationsInSecond * sizeof(float) - 1]);

        std::cout << "Magnitudes calculated: " << timer.toString() << std::endl;

        // OpenCL
        // stage: copy magnitudes values to final OpenGL buffer
        // m_settings.fftCalculator->copyMagnitudesTo(
        //  openglOpenclBuffer, static_cast<cl_uint>(elementOffsetInBuffer), &maxMagnitudeLocal);
        // spdlog::trace("Magnitudes copied: {}", timer.toString());
        
        // CUDA
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, openglOpenclBuffer);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, elementOffsetInBuffer * sizeof(float), m_settings.audioData.getSampleRate() / m_settings.fftCalculationsInSecond * sizeof(float), magnitudes);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        std::cout << "Magnitudes copied: " << timer.toString() << std::endl;

        // TODO add mutex?
        m_settings.heatmapContainer->tryUpdateMaxValue(maxMagnitudeLocal);
        m_settings.heatmapContainer->setLastFilledColumn(calculationInputData.columnIndex);

        // stage: apply the calculated values to the RTSA heatmap buffer:
        timer.restart();
        const auto referenceValue = std::pow(2.0f, 31.0f);
        // OpenCL
        // m_settings.rtsaUpdater->update(
        //   m_settings.fftCalculator->getMagnitudesBuffer(), m_rtsaGlBuffer, referenceValue);
        // spdlog::trace("RTSA updated: {}", timer.toString());
        // CUDA
        // -- todo --

        std::cout << "RTSA updated: " << timer.toString() << std::endl;

        // spdlog::trace("Whole spectrogram stage: {}", globalFftTimer.toString());
        std::cout << "Whole spectrogram stage: " << globalFftTimer.toString() << std::endl;

        delete[] buffer0;
        delete[] buffer1;
        delete[] omegas;
        delete[] magnitudes;
    }
}

void AudioFileTimeFrequencyWorker::startWork()
{
    m_workerThread =
      std::make_unique<std::jthread>([this](std::stop_token stopToken) { workLoop(stopToken); });
}

void AudioFileTimeFrequencyWorker::workLoop(std::stop_token stopToken)
{
    const auto sleepTime = 1.0f / m_settings.fftCalculationsInSecond;
    const auto oneFftSamplesOffset =
      m_settings.audioData.getSampleRate() / m_settings.fftCalculationsInSecond;
    const auto& sampleData = m_settings.audioData.getSampleData16(0);
    size_t columnIndex = 0;
    size_t globalSamplesOffset = 0;

    while (!stopToken.stop_requested() &&
           globalSamplesOffset + m_settings.oneFftSampleCount < sampleData.size())
    {
        std::this_thread::sleep_for(std::chrono::duration<float>(sleepTime));

        // create input data for FFT
        float* inputData = new float[m_settings.oneFftSampleCount];
        size_t globalIndex = globalSamplesOffset;
        for (size_t i = 0; i < m_settings.oneFftSampleCount; ++i)
        {
            inputData[i] = (float)sampleData[globalIndex++];
        }

        {
            std::lock_guard lock{ m_mutex };
            m_pendingDatas.push(PendingData{ columnIndex, inputData });
        }

        ++columnIndex;
        globalSamplesOffset += oneFftSamplesOffset;
    }
}
}
