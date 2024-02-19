#include <spectr/desktop_app/MockTimeFrequencyWorker.h>

#include <spectr/utils/Assert.h>

#include <chrono>
#include <thread>
#include <vector>

namespace spectr::desktop_app
{
MockTimeFrequencyWorker::MockTimeFrequencyWorker(MockTimeFrequencyWorkerSettings settings)
  : m_settings{ std::move(settings) }
{
    work();
}

void MockTimeFrequencyWorker::update()
{
    std::lock_guard<std::mutex> guard{ m_mutex };
    for (auto& column : m_pendingColumns)
    {
        addNextColumn(column);
    }
    m_pendingColumns.clear();
}

void MockTimeFrequencyWorker::work()
{
    std::thread workerThread{ &MockTimeFrequencyWorker::workLoop, this };
    workerThread.detach();
    //
}

void MockTimeFrequencyWorker::workLoop()
{
    constexpr bool isAddAtTheStartEnabled = true;
    if (isAddAtTheStartEnabled)
    {
        addPendingColumn();
    }

    const bool isWorkLoopEnabled = true;

    while (isWorkLoopEnabled)
    {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(m_settings.updatePeriodSeconds);

        {
            std::lock_guard<std::mutex> guard{ m_mutex };
            addPendingColumn();
        }
    }
}

void MockTimeFrequencyWorker::addPendingColumn()
{
    // generate data
    std::vector<float> values;
    for (size_t i = 0; i < m_settings.columnHeight; ++i)
    {
        const auto value = static_cast<float>(i) * 2.0f;
        values.push_back(value);
    }

    addPendingColumn(std::move(values));
}

void MockTimeFrequencyWorker::addPendingColumn(std::vector<float> values)
{
    m_pendingColumns.push_back(std::move(values));
}

void MockTimeFrequencyWorker::addNextColumn(const std::vector<float>& values)
{
    ASSERT(values.size() == m_settings.columnHeight);

    // TODO use OpenGL wrapper/adapter ?
    auto container = m_settings.heatmapContainer;
    auto buffer = container->getOrAllocateBuffer(m_nextColumnIndex);

    const auto columnSizeBytes = m_settings.columnHeight * sizeof(float);
    const auto offsetColumns = m_nextColumnIndex - buffer.startColumn;
    const auto offsetBytes = offsetColumns * columnSizeBytes;

    const auto dataSize = values.size() * sizeof(float);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer.ssbo);
    GLvoid* p = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, offsetBytes, dataSize, GL_MAP_WRITE_BIT);
    memcpy(p, values.data(), dataSize);
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    const auto maxValue = std::max_element(values.begin(), values.end());
    container->tryUpdateMaxValue(*maxValue);
    container->setLastFilledColumn(m_nextColumnIndex);

    // apply values to RTSA density heatmap 
    

    // m_settings.rtsaHeatmapContainer->apply(values);

    ++m_nextColumnIndex;
}
}
