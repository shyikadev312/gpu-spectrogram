#pragma once

#include <spectr/render_gl/TimeFrequencyHeatmapContainer.h>
#include <spectr/render_gl/RtsaContainer.h>

#include <chrono>
#include <memory>
#include <vector>
#include <mutex>

namespace spectr::desktop_app
{
struct MockTimeFrequencyWorkerSettings
{
    std::chrono::duration<float> updatePeriodSeconds;
    float updateWidthUnits;
    size_t columnHeight;
    std::shared_ptr<render_gl::TimeFrequencyHeatmapContainer> heatmapContainer;
    std::shared_ptr<render_gl::RtsaContainer> rtsaHeatmapContainer;
};

/**
 * @brief Mock worker that adds generated/hardcoded data.
 * @param settings 
*/
class MockTimeFrequencyWorker
{
public:
    MockTimeFrequencyWorker(MockTimeFrequencyWorkerSettings settings);

    void update();

private:
    void work();

    void workLoop();

    void addPendingColumn();

    void addPendingColumn(std::vector<float> values);

    void addNextColumn(const std::vector<float>& values);

private:
    MockTimeFrequencyWorkerSettings m_settings;
    size_t m_nextColumnIndex = 0;
    std::vector<std::vector<float>> m_pendingColumns;
    std::mutex m_mutex;
};
}
