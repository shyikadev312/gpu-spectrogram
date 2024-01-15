#include <spectr/desktop_app/AudioFileTimeFrequencyWorker.h>

#include <spectr/calc_cpu/FftCooleyTukeyRadix2.h>
#include <spectr/utils/Timer.h>

#include <spdlog/spdlog.h>

#include <iomanip>
#include <sstream>
#include <thread>

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
  : m_settings{ std::move(settings) }
{
}

void AudioFileTimeFrequencyWorker::update()
{
    // get buffer
    auto buffer = m_settings.heatmapContainer->getOrAllocateBuffer(0);

    utils::Timer timer;
    PendingData calculationInputData;

    // calculate fft
    while (true)
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

        m_settings.fftCalculator->execute(calculationInputData.values);

        spdlog::trace("FFT calculation, size: {}, time: {}",
                      calculationInputData.values.size(),
                      timer.toString());

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

        auto heatmapBuffer =
          m_settings.heatmapContainer->getOrAllocateBuffer(calculationInputData.columnIndex);

        auto bufferIt = m_glBuffers.find(heatmapBuffer.startColumn);

        cl::BufferGL openglOpenclBuffer;
        if (bufferIt == m_glBuffers.end())
        {
            openglOpenclBuffer = cl::BufferGL(
              m_settings.fftCalculator->getContext(), CL_MEM_READ_WRITE, heatmapBuffer.ssbo);
            m_glBuffers.insert({ heatmapBuffer.startColumn, openglOpenclBuffer });
        }
        else
        {
            openglOpenclBuffer = bufferIt->second;
        }

        const auto columnLocalIndex = calculationInputData.columnIndex - heatmapBuffer.startColumn;

        const auto elementOffsetInBuffer =
          columnLocalIndex * m_settings.heatmapContainer->getSettings().columnHeightElementCount;

        float maxMagnitudeLocal = 0;

        // stage: calculate and copy resulting values to buffer
        timer.restart();

        m_settings.fftCalculator->copyMagnitudesTo(
          openglOpenclBuffer, static_cast<cl_uint>(elementOffsetInBuffer), &maxMagnitudeLocal);

        spdlog::trace("Magnitudes calculated and copied: {}", timer.toString());

        // TODO add mutex?
        m_settings.heatmapContainer->tryUpdateMaxValue(maxMagnitudeLocal);

        spdlog::trace("Whole spectrogram stage: {}", globalFftTimer.toString());
    }
}

void AudioFileTimeFrequencyWorker::startWork()
{
    std::thread t(&AudioFileTimeFrequencyWorker::workLoop, this);
    t.detach();
}

void AudioFileTimeFrequencyWorker::workLoop()
{
    const auto sleepTime = 1.0f / m_settings.fftCalculationsInSecond;
    const auto oneFftSamplesOffset =
      m_settings.audioData.getSampleRate() / m_settings.fftCalculationsInSecond;
    const auto& sampleData = m_settings.audioData.getSampleData16(0);
    size_t columnIndex = 0;
    size_t globalSamplesOffset = 0;

    while (globalSamplesOffset + m_settings.oneFftSampleCount < sampleData.size())
    {
        std::this_thread::sleep_for(std::chrono::duration<float>(sleepTime));

        // create input data for FFT
        std::vector<float> inputData;
        for (size_t i = 0; i < m_settings.oneFftSampleCount; ++i)
        {
            const auto globalIndex = globalSamplesOffset + i;
            const auto value = static_cast<float>(sampleData[globalIndex]);
            inputData.push_back(value);
        }

        {
            std::lock_guard lock{ m_mutex };
            m_pendingDatas.push(PendingData{ columnIndex, std::move(inputData) });
        }

        ++columnIndex;
        globalSamplesOffset += oneFftSamplesOffset;
    }
}
}
