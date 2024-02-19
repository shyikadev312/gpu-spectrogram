#pragma once

#include <spectr/render_gl/GraphicsApi.h>
#include <spectr/render_gl/OpenGlUtils.h>

#include <vector>

namespace spectr::render_gl
{
/**
 * @brief Contains part of the heatmap data (several columns).
 */
struct TimeFrequencyHeatmapBuffer
{
    /**
     * @brief Start, inclusive.
     */
    size_t startColumn;

    /**
     * @brief End, inclusive.
     */
    size_t endColumn;

    /**
     * @brief OpenGL buffer that stores the data.
     */
    GLuint ssbo = NoBuffer;
};

struct TimeFrequencyHeatmapContainerSettings
{
    /**
     * @brief Which frequency value is the minimal value of the spectrogram. For example, it may be
     * 20 Hz or 440.5 Hz.
     */
    float frequencyOffset;

    /**
     * @brief How many frequency values are contained in the range of one hertz. This value means
     * the resolution of the spectrogram by the frequency axis.
     */
    float valuesInOneHertz;

    /**
     * @brief How many seconds takes each frequency column of values. This values means the
     * resolution of the spectrogram by the time axis.
     */
    float columnsInOneSecond;

    /**
     * @brief How many elements (values) are contained inside single column of spectrogram.
     */
    size_t columnHeightElementCount;

    /**
     * @brief How many columns are contained inside single buffer of spectrogram.
     */
    size_t singleBufferColumnCount;

    /**
     * @brief Max amount of heatmap buffers that can be stored in container.
     * @details When
     */
    size_t maxBuffersCount;
};

/**
 * @brief Contains values for rendering time-frequency heatmap.
 * @details Time-frequency heatmap is organized as a rectangle. This rectangle consists of vertical
 * columns, each column represents frequencies spectrum of signal at the specific point of time.
 * Time-frequency heatmap container is organized in a set of buffers. Each buffer contains values
 * (one or several column). Grid width and height have meaning in terms of spectrogram rendering.
 * Grid height is the frequency range of the spectrogram (in hertz). Grid width is the time range of
 * the spectrogram.
 *
 * Spectrogram can viewed as many columns of values stored in container. Container stores these
 * columns in buffers, one buffer may contain several columns.
 *
 * A column of values = frequencies range at specific point of time.
 * Container = set of OpenGL buffers.
 * Element count in buffer = ColumnCountInBuffer * ValuesCountInColumn.
 * Container element count = BufferCountInContainer * ColumnCountInBuffer * ValuesCountInColumn.
 */
class TimeFrequencyHeatmapContainer
{
public:
    TimeFrequencyHeatmapContainer(TimeFrequencyHeatmapContainerSettings settings);

    ~TimeFrequencyHeatmapContainer();

    const TimeFrequencyHeatmapContainerSettings& getSettings();

    const std::vector<TimeFrequencyHeatmapBuffer>& getBuffers() const;

    TimeFrequencyHeatmapBuffer& getOrAllocateBuffer(size_t columnIndex);

    std::vector<TimeFrequencyHeatmapBuffer> getVisibleBuffers(size_t startColumn,
                                                              size_t endColumn) const;

    /**
     * @brief Update the global maximum value stored in the container.
     * @details Container stores the global maximum value which is later used for rendering.
     */
    void tryUpdateMaxValue(float value);

    /**
     * @brief Set index of the last container column filled with data.
     * @details Since the columns are added to the container one by one, but buffer allocates memory
     * for more than one column, it is important to track how many columns already have the data, in
     * order no to render the columns without data.
     */
    void setLastFilledColumn(size_t columnIndex);

    /**
     * @brief Get max element value through whole container.
     */
    float getMaxValue() const;

    /**
     * @brief Get the min and max time values of the stored data.
     * @return
     */
    Range getTimeRange() const;

    /**
     * @brief Get the min and max frequency values of the stored data.
     */
    Range getFrequencyRange() const;

    /**
     * @brief Get one column size in world units. X - seconds, Y - hertz.
     */
    glm::vec2 getColumnSize() const;

private:
    TimeFrequencyHeatmapContainerSettings m_settings;
    std::vector<TimeFrequencyHeatmapBuffer> m_buffers;
    float m_globalMaxValue = 0;
    size_t m_lastFilledColumn = 0;
};
}
