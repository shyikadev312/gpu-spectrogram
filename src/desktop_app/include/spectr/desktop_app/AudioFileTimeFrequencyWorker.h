#pragma once

#include <spectr/calc_opencl/FftCooleyTukeyRadix2.h>
#include <spectr/render_gl/TimeFrequencyHeatmapContainer.h>

#include <spectr/audio_loader/AudioData.h>

#include <memory>
#include <queue>
#include <vector>
#include <mutex>
#include <unordered_map>

namespace spectr::desktop_app
{
struct AudioFileTimeFrequencyWorkerSettings
{
    audio_loader::AudioData audioData;
    size_t oneFftSampleCount;
    size_t fftCalculationsInSecond;
    std::shared_ptr<render_gl::TimeFrequencyHeatmapContainer> heatmapContainer;
    std::unique_ptr<calc_opencl::FftCooleyTukeyRadix2> fftCalculator;
};

struct PendingData
{
    size_t columnIndex;
    std::vector<float> values;
};

class AudioFileTimeFrequencyWorker
{
public:
    AudioFileTimeFrequencyWorker(AudioFileTimeFrequencyWorkerSettings settings);

    void startWork();

    void update();

private:
    void workLoop();

private:
    AudioFileTimeFrequencyWorkerSettings m_settings;
    std::queue<PendingData> m_pendingDatas;
    std::mutex m_mutex;
    std::unordered_map<size_t, cl::BufferGL> m_glBuffers;
};
}
