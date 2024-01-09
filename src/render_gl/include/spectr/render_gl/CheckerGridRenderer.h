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
    float m_cellSize = 1;
    GLuint m_shaderProgram = NoShaderProgram;
    GLuint m_vbo = NoBuffer;
    GLuint m_vao = NoBuffer;
};
}
