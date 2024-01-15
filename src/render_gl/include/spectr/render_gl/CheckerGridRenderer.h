#pragma once

#include <spectr/render_gl/OpenGlUtils.h>
#include <spectr/render_gl/RenderContext.h>

namespace spectr::render_gl
{
class CheckerGridRenderer
{
public:
    CheckerGridRenderer();

    ~CheckerGridRenderer();

    void render(const RenderContext& renderContext);

private:
    glm::vec2 m_cellSize = { 1.0f, 1.0f };
    GLuint m_vbo = NoBuffer;
    GLuint m_vao = NoBuffer;
    GLuint m_shaderProgram = NoShaderProgram;
    GLint m_cellSizeScaleIdx = NoUniform;
};
}
