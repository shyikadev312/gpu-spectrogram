#pragma once

#include <spectr/render_gl/TimeFrequencyHeatmapContainer.h>

#include <spectr/render_gl/RenderContext.h>

#include <memory>

namespace spectr::render_gl
{
enum class WaterfallDirection
{
    Horizontal,
    Vertical
};

enum class MagnitudeColorMode
{
    Grayscale,
    FiveColorScale,
};

enum class MagnitudeAxisScale
{
    Linear,
    Logarithmic,
};

enum class FrequencyAxisScale
{
    Linear,
    Logarithmic,
    // Mel // mel spectrogram
};

enum class MagnitudeInterpolationMode
{
    Sharp,
    Bilinear,
};

std::string toString(WaterfallDirection waterfallDirection);
std::string toString(MagnitudeColorMode colorMode);
std::string toString(MagnitudeAxisScale magnitudeAxisScale);
std::string toString(FrequencyAxisScale frequencyAxisScale);
std::string toString(MagnitudeInterpolationMode magnitudeInterpolationMode);

struct LogScaleUtils
{
    static float getFrequency(float y, float maxFrequency);
};

class TimeFrequencyHeatmapRenderer
{
public:
    TimeFrequencyHeatmapRenderer(
      std::shared_ptr<render_gl::TimeFrequencyHeatmapContainer> timeFrequencyHeatmapContainer);

    ~TimeFrequencyHeatmapRenderer();

    void render(const RenderContext& renderContext);

    float getScaleMinValue() const;

    void setScaleMinValue(float newMinValue);

    float getScaleMaxValue() const;

    void setScaleMaxValue(float newMaxValue);

    void resetScaleRange();

    void setMagnitudeColorMode(MagnitudeColorMode magnitudeColorMode);

    void setMagnitudeAxisScale(MagnitudeAxisScale magnitudeAxisScale);

    void setFrequencyAxisScale(FrequencyAxisScale frequencyAxisScale);

    void setMagnitudeInterpolationMode(MagnitudeInterpolationMode magnitudeInterpolationMode);

    void setWaterfallDirectionMode(WaterfallDirection waterfallDirection);

    glm::mat3 getRotationMatrix() const;

private:
    void recreateRenderProgram();

private:
    std::shared_ptr<TimeFrequencyHeatmapContainer> m_container;
    GLuint m_quadVbo = NoBuffer;
    GLuint m_quadVao = NoBuffer;
    GLuint m_heatmapShaderProgram = NoShaderProgram;
    float m_scaleMinValue = 0.0f;
    float m_scaleMaxValue = 1.0;

    MagnitudeColorMode m_magnitudeColorMode = MagnitudeColorMode::FiveColorScale;
    MagnitudeAxisScale m_magnitudeAxisScale = MagnitudeAxisScale::Linear;
    FrequencyAxisScale m_frequencyAxisScale = FrequencyAxisScale::Linear;
    MagnitudeInterpolationMode m_magnitudeInterpolationMode = MagnitudeInterpolationMode::Sharp;
    WaterfallDirection m_waterfallDirection = WaterfallDirection::Horizontal;

    // Heatmap shader uniforms indices:
    GLint m_localToWorldIdx = NoUniform;
    GLint m_localToClipIdx = NoUniform;
    GLint m_lowerLeftIdx = NoUniform;
    GLint m_columnWidthUnitsIdx = NoUniform;
    GLint m_valueHeightUnitsIdx = NoUniform;
    GLint m_columnHeightValuesIdx = NoUniform;
    GLint m_minValueIdx = NoUniform;
    GLint m_maxValueIdx = NoUniform;
    // GLint m_Idx = NoUniform;
};
}
