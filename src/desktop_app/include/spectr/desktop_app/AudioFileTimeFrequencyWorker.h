#pragma once

#include <spectr/calc_opencl/FftCooleyTukeyRadix2CL.h>
#include <spectr/calc_opencl/RtsaUpdater.h>
#include <spectr/render_gl/RtsaContainer.h>
#include <spectr/render_gl/TimeFrequencyHeatmapContainer.h>

#include <spectr/audio_loader/SignalData.h>

#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <unordered_map>
#include <vector>

namespace spectr::desktop_app
{
struct AudioFileTimeFrequencyWorkerSettings
{
    audio_loader::SignalData audioData;
    size_t oneFftSampleCount;
    size_t fftCalculationsInSecond;
    std::shared_ptr<render_gl::TimeFrequencyHeatmapContainer> heatmapContainer;
    std::shared_ptr<render_gl::RtsaContainer> rtsaHeatmapContainer;
    std::unique_ptr<calc_opencl::FftCooleyTukeyRadix2> fftCalculator;
    std::unique_ptr<calc_opencl::RtsaUpdater> rtsaUpdater;
};

struct PendingData
{
    size_t columnIndex;
    float* values;
};

class AudioFileTimeFrequencyWorker
{
public:
    AudioFileTimeFrequencyWorker(AudioFileTimeFrequencyWorkerSettings settings);

    ~AudioFileTimeFrequencyWorker();

    void startWork();

    void update();

private:
    void workLoop(std::stop_token stoken);

private:
    AudioFileTimeFrequencyWorkerSettings m_settings;
    std::unique_ptr<std::jthread> m_workerThread;
    std::queue<PendingData> m_pendingDatas;
    std::mutex m_mutex;
    std::unordered_map<size_t, cl::BufferGL> m_glBuffers;
    cl::BufferGL m_rtsaGlBuffer;
};
}
