#include <spectr/render_gl/RtsaRenderer.h>

#include <spectr/render_gl/OpenGlUtils.h>
#include <spectr/utils/Asset.h>
#include <spectr/utils/File.h>

#include <glm/gtc/type_ptr.hpp>

namespace spectr::render_gl
{
namespace
{
// Quad of size 1x1 with center in point (0;0)
const std::vector<float> QuadVertices{
    -0.5f, -0.5f, 0.0f, // left-down
    0.5f,  -0.5f, 0.0f, // right-down
    0.5f,  0.5f,  0.0f, // right-top

    -0.5f, -0.5f, 0.0f, // left-down
    0.5f,  0.5f,  0.0f, // right-top
    -0.5f, 0.5f,  0.0f, // left-top
};

const auto RtsaVertexShaderPath = "shaders/rtsa/Vertex.glsl";
const auto RtsaFragmentShaderPath = "shaders/rtsa/Fragment.glsl";
const auto IntensityScaleShaderPath = "shaders/utils/IntensityColoredScale.glsl";
}

RtsaRenderer::RtsaRenderer(std::shared_ptr<RtsaContainer> container)
  : m_container{ container }
{
    // create shader program
    std::vector<std::string> vertexSources;
    std::vector<std::string> fragmentSources;

    const auto vertexSource = utils::File::read(utils::Asset::getPath(RtsaVertexShaderPath));
    vertexSources.push_back(vertexSource);

    const auto fragmentSource = utils::File::read(utils::Asset::getPath(RtsaFragmentShaderPath));
    const auto intensityScaleSource =
      utils::File::read(utils::Asset::getPath(IntensityScaleShaderPath));

    fragmentSources.push_back("#version 430 core\n");
    fragmentSources.push_back(intensityScaleSource);
    fragmentSources.push_back(fragmentSource);

    m_heatmapShaderProgram = OpenGlUtils::createShaderProgram(vertexSources, fragmentSources);

    // get uniforms indices
    m_columnSizeIndex = glGetUniformLocation(m_heatmapShaderProgram, "columnSize");
    m_localToClipIndex = glGetUniformLocation(m_heatmapShaderProgram, "localToClip");
    m_localToWorldIndex = glGetUniformLocation(m_heatmapShaderProgram, "localToWorld");
    m_dbfsToMagnitudeCellCoeffIndex =
      glGetUniformLocation(m_heatmapShaderProgram, "dbfsToMagnitudeCellCoeff");

    // create VBO and VAO
    glGenBuffers(1, &m_quadVbo);
    glGenVertexArrays(1, &m_quadVao);

    glBindVertexArray(m_quadVao);

    glBindBuffer(GL_ARRAY_BUFFER, m_quadVbo);
    glBufferData(
      GL_ARRAY_BUFFER, QuadVertices.size() * sizeof(float), QuadVertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, NoBuffer);
    glBindVertexArray(NoBuffer);
}

RtsaRenderer::~RtsaRenderer()
{
    glDeleteVertexArrays(1, &m_quadVao);
    glDeleteBuffers(1, &m_quadVbo);
    glDeleteProgram(m_heatmapShaderProgram);
}

void RtsaRenderer::render(const render_gl::RenderContext& renderContext)
{
    // set rendering parameters
    glUseProgram(m_heatmapShaderProgram);

    const auto& settings = m_container->getSettings();

    const auto width = settings.getMaxFrequency();
    const auto height = settings.magnitudeDecibelRange;

    const glm::mat3 localToWorldMatrix{ glm::vec3{ width, 0, 0 },
                                        glm::vec3{ 0, height, 0 },
                                        glm::vec3{ width / 2.0f, -(height / 2.0f), 1 } };

    const auto localToClipMatrix = renderContext.camera->getViewProjection() * localToWorldMatrix;

    const auto frequencyOffset = 0;

    // const glm::mat3 localToBufferMatrix{
    const glm::mat3 localToBufferMatrix{
        glm::vec3{ settings.frequencyValuesCount, 0, 0 },
        glm::vec3{ 0, height, 0 },
        glm::vec3{ settings.frequencyValuesCount / 2.0f + frequencyOffset, -(height / 2.0f), 1 }
    };

    glUniformMatrix3fv(m_localToClipIndex, 1, GL_FALSE, glm::value_ptr(localToClipMatrix));
    glUniformMatrix3fv(m_localToWorldIndex, 1, GL_FALSE, glm::value_ptr(localToBufferMatrix));

    // set OpenGL uniform/SSBO buffer
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_container->getBuffer());

    glUniform1ui(m_columnSizeIndex,
                 static_cast<GLuint>(m_container->getSettings().magnitudeRangeValuesCount));

    const auto dbfsToMagnitudeCellCoeff =
      settings.magnitudeRangeValuesCount / settings.magnitudeDecibelRange;
    glUniform1f(m_dbfsToMagnitudeCellCoeffIndex, dbfsToMagnitudeCellCoeff);

    // draw quad
    glBindVertexArray(m_quadVao);
    glBindBuffer(GL_ARRAY_BUFFER, m_quadVbo);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindBuffer(GL_ARRAY_BUFFER, NoBuffer);
    glBindVertexArray(NoBuffer);
}
}
