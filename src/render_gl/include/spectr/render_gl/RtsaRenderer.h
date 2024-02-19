#pragma once

#include <spectr/render_gl/Camera.h>
#include <spectr/render_gl/GraphicsApi.h>
#include <spectr/render_gl/RenderContext.h>
#include <spectr/render_gl/RtsaContainer.h>

#include <memory>

namespace spectr::render_gl
{
class RtsaRenderer
{
public:
    RtsaRenderer(std::shared_ptr<RtsaContainer> container);

    ~RtsaRenderer();

    void render(const render_gl::RenderContext& renderContext);

private:
    std::shared_ptr<RtsaContainer> m_container;
    GLuint m_quadVbo = NoBuffer;
    GLuint m_quadVao = NoBuffer;
    GLuint m_heatmapShaderProgram = NoShaderProgram;
    GLint m_columnSizeIndex = NoUniform;
    GLint m_localToClipIndex = NoUniform;
    GLint m_localToWorldIndex = NoUniform;
    GLint m_dbfsToMagnitudeCellCoeffIndex = NoUniform;
};
}
