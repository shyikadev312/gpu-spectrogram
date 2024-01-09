#pragma once

#include <spectr/render_gl/GraphicsApi.h>
#include <spectr/render_gl/OpenGlUtils.h>

#include <vector>

namespace spectr::render_gl
{
struct TimeFrequencyHeatmapBuffer
{
    /**
     * @brief Start, inclusive.
     */
    size_t startColumn;

    /**
     * @brief End, incluse
     */
    size_t endColumn;

    /**
     * @brief OpenGL buffer
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
};

/**
 * @brief Contains values for rendering time-frequency heatmap.
 * @details Time-frequency heatmap container is organized in a set of buffers. Each buffer contains
 * values. All values form a rectangular grid. Grid width and height have meaning in terms of
 * spectrogram rendering. Grid height is the frequency range of the spectrogram (in hertz). Grid
 * width is the time range of the spectrogram.
 *
 * Spectrogram can viewed as many columns of values stored in container. Container stores these
 * columns in buffers, one buffer may contain several columns.
 *
 * A column of values = frequency range.
 * Container = OpenGL buffers.
 * Buffer element count = N * column.
 */
class TimeFrequencyHeatmapContainer
{
public:
    TimeFrequencyHeatmapContainer(TimeFrequencyHeatmapContainerSettings settings);

    ~TimeFrequencyHeatmapContainer();

    const TimeFrequencyHeatmapContainerSettings& getSettings();

    const std::vector<TimeFrequencyHeatmapBuffer>& getBuffers() const;

    TimeFrequencyHeatmapBuffer& getOrAllocateBuffer(size_t columnIndex);

    std::vector<TimeFrequencyHeatmapBuffer> getVisibleBuffers(size_t startColumn, size_t endColumn) const;

private:
    TimeFrequencyHeatmapContainerSettings m_settings;
    std::vector<TimeFrequencyHeatmapBuffer> m_buffers;
};
}
