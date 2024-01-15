#include <spectr/render_gl/TimeFrequencyHeatmapContainer.h>

#include <spectr/utils/Assert.h>
#include <spectr/utils/Exception.h>

namespace spectr::render_gl
{
TimeFrequencyHeatmapContainer::TimeFrequencyHeatmapContainer(
  TimeFrequencyHeatmapContainerSettings settings)
  : m_settings{ std::move(settings) }
{
}

TimeFrequencyHeatmapContainer::~TimeFrequencyHeatmapContainer() {}

const TimeFrequencyHeatmapContainerSettings& TimeFrequencyHeatmapContainer::getSettings()
{
    return m_settings;
}

const std::vector<TimeFrequencyHeatmapBuffer>& TimeFrequencyHeatmapContainer::getBuffers() const
{
    return m_buffers;
}

TimeFrequencyHeatmapBuffer& TimeFrequencyHeatmapContainer::getOrAllocateBuffer(size_t columnIndex)
{
    // TODO binary search
    for (auto& buffer : m_buffers)
    {
        if (columnIndex >= buffer.startColumn && columnIndex <= buffer.endColumn)
        {
            return buffer;
        }
    }

    // buffer not found, it means it's not created yet, so create new buffer
    const auto bufferGlobalIndex = columnIndex / m_settings.singleBufferColumnCount;
    const auto bufferStartColumn = bufferGlobalIndex * m_settings.singleBufferColumnCount;
    const auto bufferEndColumn = bufferStartColumn + m_settings.singleBufferColumnCount - 1;

    GLuint ssbo = 0;
    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);

    const auto columnSizeBytes = sizeof(float) * m_settings.columnHeightElementCount;
    const auto bufferSizeBytes = columnSizeBytes * m_settings.singleBufferColumnCount;

    // std::vector<float> vals;
    // vals.resize(200, 5);
    // glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSizeBytes, vals.data(), GL_DYNAMIC_COPY);
    glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSizeBytes, nullptr, GL_DYNAMIC_COPY);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    TimeFrequencyHeatmapBuffer buffer{ bufferStartColumn, bufferEndColumn, ssbo };
    m_buffers.push_back(buffer);

    return m_buffers.back();
}

std::vector<TimeFrequencyHeatmapBuffer> TimeFrequencyHeatmapContainer::getVisibleBuffers(
  size_t startColumn,
  size_t endColumn) const
{
    std::vector<TimeFrequencyHeatmapBuffer> visibleBuffers;
    for (const auto& buffer : m_buffers)
    {
        if (buffer.endColumn >= startColumn && buffer.startColumn <= endColumn)
        {
            visibleBuffers.push_back(buffer);
        }
    }

    return visibleBuffers;
}

void TimeFrequencyHeatmapContainer::tryUpdateMaxValue(float value)
{
    if (value > m_globalMaxValue)
    {
        m_globalMaxValue = value;
    }
}

float TimeFrequencyHeatmapContainer::getMaxValue() const
{
    return m_globalMaxValue;
}
}
