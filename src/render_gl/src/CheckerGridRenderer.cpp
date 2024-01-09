#include <spectr/render_gl/CheckerGridRenderer.h>

#include <spectr/utils/Asset.h>
#include <spectr/utils/File.h>

#include <vector>

namespace spectr::render_gl
{
namespace
{
const auto checkerVertexShaderPath = "shaders/checker_grid/Vertex.glsl";
const auto checkerFragmentShaderPath = "shaders/checker_grid/Fragment.glsl";

const std::vector<float> vertices{
    -1.0f, -1.0f, 0.0f, // left-down
    1.0f,  -1.0f, 0.0f, // right-down
    1.0f,  1.0f,  0.0f, // right-top

    -1.0f, -1.0f, 0.0f, // left-down
    1.0f,  1.0f,  0.0f, // right-top
    -1.0f, 1.0f,  0.0f, // left-top
};
}

CheckerGridRenderer::CheckerGridRenderer()
{
    const auto vertexShaderSource =
      utils::File::read(utils::Asset::getPath(checkerVertexShaderPath));

    const auto fragmentShaderSource =
      utils::File::read(utils::Asset::getPath(checkerFragmentShaderPath));

    m_shaderProgram = OpenGlUtils::createShaderProgram(vertexShaderSource, fragmentShaderSource);

    glGenBuffers(1, &m_vbo);
    glGenVertexArrays(1, &m_vao);

    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, NoBuffer);
    glBindVertexArray(NoBuffer);
}

CheckerGridRenderer::~CheckerGridRenderer()
{
    glDeleteVertexArrays(1, &m_vao);
    glDeleteBuffers(1, &m_vbo);
    glDeleteProgram(m_shaderProgram);
}

void CheckerGridRenderer::render(const RenderContext& renderContext)
{
    glUseProgram(m_shaderProgram);

    const auto mvpInvId = glGetUniformLocation(m_shaderProgram, "mvpInv");
    const auto mvp = renderContext.camera->getMvpInv();
    const auto mvpPtr = &mvp[0][0];
    glUniformMatrix3fv(mvpInvId, 1, GL_FALSE, mvpPtr);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindBuffer(GL_ARRAY_BUFFER, NoBuffer);
    glBindVertexArray(NoBuffer);
}
}
