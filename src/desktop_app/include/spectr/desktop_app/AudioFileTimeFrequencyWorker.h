#pragma once

#include <spectr/calc_opencl/FftCooleyTukeyRadix2CL.h>
#include <spectr/calc_opencl/RtsaUpdater.h>
#include <spectr/render_gl/RtsaContainer.h>
#include <spectr/render_gl/TimeFrequencyHeatmapContainer.h>
#include <spectr/real_time_input/RealTimeInput.h>

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
    std::shared_ptr<real_time_input::RealTimeInput> source;
    audio_loader::SignalData audioData;
    size_t oneFftSampleCount;
    size_t fftCalculationsInSecond;
    std::shared_ptr<render_gl::TimeFrequencyHeatmapContainer> heatmapContainer;
    std::shared_ptr<render_gl::RtsaContainer> rtsaHeatmapContainer;
    std::unique_ptr<calc_opencl::FftCooleyTukeyRadix2> fftCalculator;
    std::unique_ptr<calc_opencl::RtsaUpdater> rtsaUpdater;
    size_t rtsaBufferSize;
    size_t fftSize;
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

    size_t bufferSize; // temporary quick and dirty fix

private:
    AudioFileTimeFrequencyWorkerSettings m_settings;
    std::unique_ptr<std::jthread> m_workerThread;
    std::queue<PendingData> m_pendingDatas;
    std::mutex m_mutex;
    std::unordered_map<size_t, GLuint> m_glBuffers;
    GLuint m_rtsaGlBuffer;
};
}
