#pragma once

#include <spectr/render_gl/TimeFrequencyHeatmapContainer.h>

#include <spectr/render_gl/RenderContext.h>

#include <memory>

namespace spectr::render_gl
{
class TimeFrequencyHeatmapRenderer
{
public:
    TimeFrequencyHeatmapRenderer(
      std::shared_ptr<render_gl::TimeFrequencyHeatmapContainer> timeFrequencyHeatmapContainer);

    ~TimeFrequencyHeatmapRenderer();

    void render(RenderContext& renderContext);

    float getScaleMinValue() const;

    void setScaleMinValue(float newMinValue);

    float getScaleMaxValue() const;

    void setScaleMaxValue(float newMaxValue);

    void resetScaleRange();

private:
    std::shared_ptr<TimeFrequencyHeatmapContainer> m_container;
    GLuint m_quadVbo = NoBuffer;
    GLuint m_quadVao = NoBuffer;
    GLuint m_heatmapShaderProgram = NoShaderProgram;
    float m_scaleMinValue = 0.0f;
    float m_scaleMaxValue = 125734344; // 1 << 15;

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
