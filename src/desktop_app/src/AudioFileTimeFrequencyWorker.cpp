#include <spectr/desktop_app/AudioFileTimeFrequencyWorker.h>

#include <span>
#include <thread>

namespace spectr::desktop_app
{
AudioFileTimeFrequencyWorker::AudioFileTimeFrequencyWorker(
  AudioFileTimeFrequencyWorkerSettings settings)
  : m_settings{ std::move(settings) }
{
}

void AudioFileTimeFrequencyWorker::update()
{
    // get buffer
    auto buffer = m_settings.heatmapContainer->getOrAllocateBuffer(0);

    // calculate fft
    while (!m_pendingDatas.empty())
    {
        const auto& calculationInputData = m_pendingDatas.front();
        m_settings.fftCalculator->execute(calculationInputData.values);

        auto heatmapBuffer =
          m_settings.heatmapContainer->getOrAllocateBuffer(calculationInputData.columnIndex);

        cl::BufferGL openglOpenclBuffer(
          m_settings.fftCalculator->getContext(), CL_MEM_READ_WRITE, heatmapBuffer.ssbo);

        /*const auto bufferIndex = heatmapBuffer.startColumn /
                                 m_settings.heatmapContainer->getSettings().singleBufferColumnCount;*/

        const auto columnLocalIndex = heatmapBuffer.startColumn - calculationInputData.columnIndex;

        const auto elementOffsetInBuffer =
          columnLocalIndex * m_settings.heatmapContainer->getSettings().columnHeightElementCount;

        m_settings.fftCalculator->copyFrequenciesTo(openglOpenclBuffer,
                                                    static_cast<cl_uint>(elementOffsetInBuffer));

        m_pendingDatas.pop();
    }

    // copy resulting values to buffer
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

    while (globalSamplesOffset + oneFftSamplesOffset < sampleData.size())
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

        m_pendingDatas.push(PendingData{ columnIndex, std::move(inputData) });

        ++columnIndex;
        globalSamplesOffset += m_settings.oneFftSampleCount;
    }
}
}
