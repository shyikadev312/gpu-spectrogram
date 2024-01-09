#include <spectr/render_gl/TimeFrequencyHeatmapRenderer.h>

#include <spectr/render_gl/GraphicsApi.h>
#include <spectr/render_gl/OpenGlUtils.h>
#include <spectr/render_gl/TimeFrequencyHeatmapContainer.h>

#include <spectr/utils/Asset.h>
#include <spectr/utils/File.h>

namespace spectr::render_gl
{
namespace
{
const auto HeatmapVertexShaderPath = "shaders/time_frequency_heatmap/Vertex.glsl";
const auto HeatmapFragmentShaderPath = "shaders/time_frequency_heatmap/Fragment.glsl";

// Quad of size 1x1 with the center in the coordinates origin point.
const std::vector<float> QuadVertices{
    -0.5f, -0.5f, 0.0f, // left-down
    0.5f,  -0.5f, 0.0f, // right-down
    0.5f,  0.5f,  0.0f, // right-top

    -0.5f, -0.5f, 0.0f, // left-down
    0.5f,  0.5f,  0.0f, // right-top
    -0.5f, 0.5f,  0.0f, // left-top
};
}

TimeFrequencyHeatmapRenderer::TimeFrequencyHeatmapRenderer(
  std::shared_ptr<render_gl::TimeFrequencyHeatmapContainer> timeFrequencyHeatmapContainer)
  : m_container{ timeFrequencyHeatmapContainer }
{
    // create shader
    const auto vertexShaderSource =
      utils::File::read(utils::Asset::getPath(HeatmapVertexShaderPath));
    const auto fragmentShaderSource =
      utils::File::read(utils::Asset::getPath(HeatmapFragmentShaderPath));
    m_heatmapShaderProgram =
      OpenGlUtils::createShaderProgram(vertexShaderSource, fragmentShaderSource);

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

    // get uniforms indices
    m_localToWorldIdx = glGetUniformLocation(m_heatmapShaderProgram, "localToWorld");
    m_localToClipIdx = glGetUniformLocation(m_heatmapShaderProgram, "localToClip");
    m_lowerLeftIdx = glGetUniformLocation(m_heatmapShaderProgram, "lowerLeft");
    m_columnWidthUnitsIdx = glGetUniformLocation(m_heatmapShaderProgram, "columnWidthUnits");
    m_valueHeightUnitsIdx = glGetUniformLocation(m_heatmapShaderProgram, "valueHeightUnits");
    m_columnHeightValuesIdx = glGetUniformLocation(m_heatmapShaderProgram, "columnHeightValues");
}

TimeFrequencyHeatmapRenderer::~TimeFrequencyHeatmapRenderer()
{
    //
}

void TimeFrequencyHeatmapRenderer::render(RenderContext& renderContext)
{
    const auto contSettings = m_container->getSettings();

    const auto lowerLeft = renderContext.ndcToWorld({ -1, -1 });
    const auto upperRight = renderContext.ndcToWorld({ 1, 1 });

    const auto startX = lowerLeft.x;
    const auto endX = upperRight.x;

    const auto startColumn = startX / contSettings.columnsInOneSecond;
    const auto endColumn = endX / contSettings.columnsInOneSecond;

    const auto visibleHeatmapBuffers =
      m_container->getVisibleBuffers(static_cast<size_t>(std::max(0.0f, startColumn)),
                                     static_cast<size_t>(std::max(0.0f, endColumn)));

    const auto bufferWidthSeconds =
      contSettings.singleBufferColumnCount / contSettings.columnsInOneSecond;
    const auto bufferHeightHertz =
      contSettings.columnHeightElementCount / contSettings.valuesInOneHertz;

    const auto constantHeightOffset = contSettings.frequencyOffset;

    for (const auto& heatmapBuffer : visibleHeatmapBuffers)
    {
        // render heatmap chunk:
        const auto bufferStartSeconds = heatmapBuffer.startColumn * contSettings.columnsInOneSecond;

        const glm::mat3 worldMatrix{ { bufferWidthSeconds, 0, 0 },
                                     { 0, bufferHeightHertz, 0 },
                                     { bufferStartSeconds + bufferWidthSeconds / 2.0f,
                                       contSettings.frequencyOffset + bufferHeightHertz / 2.0f,
                                       1 } };

        // auto pos = worldMatrix * glm::vec3(0, 0, 1);

        // set rendering parameters
        glUseProgram(m_heatmapShaderProgram);

        // set local to clip matrix
        const auto localToClipMat = renderContext.camera->getMvp() * worldMatrix;
        const auto localToClipPtr = &localToClipMat[0][0];
        glUniformMatrix3fv(m_localToClipIdx, 1, GL_FALSE, localToClipPtr);

        // set local to world matrix
        const auto localToWorldPtr = &worldMatrix[0][0];
        glUniformMatrix3fv(m_localToWorldIdx, 1, GL_FALSE, localToWorldPtr);

        // set OpenGL uniform/SSBO buffer
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, heatmapBuffer.ssbo);

        const auto columnWidthSeconds = 1.0f / contSettings.columnsInOneSecond;

        const auto bufferLeftEdgeOffsetX = heatmapBuffer.startColumn * columnWidthSeconds;
        glUniform2f(m_lowerLeftIdx, bufferLeftEdgeOffsetX, constantHeightOffset);

        glUniform1f(m_columnWidthUnitsIdx, columnWidthSeconds);

        const auto singleColumnElementHeight = 1.0f / contSettings.valuesInOneHertz;
        glUniform1f(m_valueHeightUnitsIdx, singleColumnElementHeight);

        glUniform1ui(m_columnHeightValuesIdx, contSettings.columnHeightElementCount);

        // draw quad
        glBindVertexArray(m_quadVao);
        glBindBuffer(GL_ARRAY_BUFFER, m_quadVbo);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindBuffer(GL_ARRAY_BUFFER, NoBuffer);
        glBindVertexArray(NoBuffer);
    }
}
}
