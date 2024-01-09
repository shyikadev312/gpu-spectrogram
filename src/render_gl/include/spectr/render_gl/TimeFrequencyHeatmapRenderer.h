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

private:
    std::shared_ptr<TimeFrequencyHeatmapContainer> m_container;
    GLuint m_quadVbo = NoBuffer;
    GLuint m_quadVao = NoBuffer;
    GLuint m_heatmapShaderProgram = NoShaderProgram;

    // Heatmap shader uniforms indices:
    GLint m_localToWorldIdx = NoUniform;
    GLint m_localToClipIdx = NoUniform;
    GLint m_lowerLeftIdx = NoUniform;
    GLint m_columnWidthUnitsIdx = NoUniform;
    GLint m_valueHeightUnitsIdx = NoUniform;
    GLint m_columnHeightValuesIdx = NoUniform;
    // GLint m_Idx = NoUniform;
};
}
