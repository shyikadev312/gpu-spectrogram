#pragma once

#include <spectr/render_gl/GraphicsApi.h>
#include <spectr/render_gl/OpenGlUtils.h>

namespace spectr::render_gl
{
/**
 * @brief
 * @details
 */
class RtsaContainerSettings
{
public:
    /**
     * @brief The resolution of the view by width (x coordinate - frequency axis).
     */
    size_t frequencyValuesCount;

    /**
     * @brief Max value of the decibel range (y coordinate - magnitude axis).
     */
    float magnitudeDecibelRange;

    /**
     * @brief Resolution of the view by height (y coordinate - magnitude axis).
     */
    size_t magnitudeRangeValuesCount;

    float valuesInOneHertz;

    float getMaxFrequency() const;
};

/**
 * @brief
 * @details
 */
class RtsaContainer
{
public:
    RtsaContainer(RtsaContainerSettings settings);

    ~RtsaContainer();

    GLuint getBuffer() const;

    const RtsaContainerSettings& getSettings() const;

private:
    RtsaContainerSettings m_settings;
    GLuint m_rtsaHeatmapSsbo = NoBuffer;
};
}
