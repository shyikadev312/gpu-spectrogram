#include <spectr/render_gl/TimeFrequencyHeatmapRenderer.h>

#include <spectr/render_gl/GraphicsApi.h>
#include <spectr/render_gl/OpenGlUtils.h>
#include <spectr/render_gl/TimeFrequencyHeatmapContainer.h>

#include <spectr/utils/Asset.h>
#include <spectr/utils/Exception.h>
#include <spectr/utils/File.h>

#include <glm/gtc/type_ptr.hpp>

namespace spectr::render_gl
{
namespace
{
const auto HeatmapVertexShaderPath = "shaders/time_frequency_heatmap/Vertex.glsl";
const auto HeatmapFragmentShaderPath = "shaders/time_frequency_heatmap/Fragment.glsl";
const auto MagnitudeGrayScaleShaderPath = "shaders/utils/IntensityGrayScale.glsl";
const auto MagnitudeColoredScaleShaderPath = "shaders/utils/IntensityColoredScale.glsl";
const auto FrequencyLinearScaleShaderPath = "shaders/utils/FrequencyLinearScale.glsl";
const auto FrequencyLogarithmicScaleShaderPath = "shaders/utils/FrequencyLogarithmicScale.glsl";
const auto MagnitudeScaleLinearShaderPath =
  "shaders/time_frequency_heatmap/MagnitudeLinearScale.glsl";
const auto MagnitudeScaleLogarithmicShaderPath =
  "shaders/time_frequency_heatmap/MagnitudeLogarithmicScale.glsl";

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

std::string toString(WaterfallDirection waterfallDirection)
{
    switch (waterfallDirection)
    {
        case WaterfallDirection::Horizontal: return "Horizontal";
        case WaterfallDirection::Vertical: return "Vertical";
        default:
            throw utils::Exception("Unrecognized waterfall direction: {}",
                                   static_cast<int>(waterfallDirection));
    }
}

std::string toString(MagnitudeColorMode colorMode)
{
    switch (colorMode)
    {
        case MagnitudeColorMode::Grayscale: return "Grayscale";
        case MagnitudeColorMode::FiveColorScale: return "Colored";
        default: throw utils::Exception("Unrecognized color mode: {}", static_cast<int>(colorMode));
    }
}

std::string toString(MagnitudeAxisScale magnitudeAxisScale)
{
    switch (magnitudeAxisScale)
    {
        case spectr::render_gl::MagnitudeAxisScale::Linear: return "Linear";
        case spectr::render_gl::MagnitudeAxisScale::Logarithmic: return "Logarithmic";
        default:
            throw utils::Exception("Unrecognized magnitude scale: {}",
                                   static_cast<int>(magnitudeAxisScale));
    }
}

std::string toString(FrequencyAxisScale frequencyAxisMode)
{
    switch (frequencyAxisMode)
    {
        case FrequencyAxisScale::Linear: return "Linear";
        case FrequencyAxisScale::Logarithmic: return "Logarithmic";
        default:
            throw utils::Exception("Unrecognized frequency axis mode: {}",
                                   static_cast<int>(frequencyAxisMode));
    }
}

std::string toString(MagnitudeInterpolationMode magnitudeInterpolationMode)
{
    switch (magnitudeInterpolationMode)
    {
        case MagnitudeInterpolationMode::Sharp: return "Sharp";
        case MagnitudeInterpolationMode::Bilinear: return "Bilinear";
        default:
            throw utils::Exception("Unrecognized heatmap magnitude interpolation mode: {}",
                                   static_cast<int>(magnitudeInterpolationMode));
    }
}

float LogScaleUtils::getFrequency(float y, float maxFrequency)
{
    const float minY = log10(0.0f + 1.0f);
    const float maxY = log10(maxFrequency + 1.0f);
    const float coeff = maxFrequency / maxY;
    const float mappedY = y / coeff;
    const float frequencyValue = pow(10.0f, mappedY) - 1.0f;
    return frequencyValue;
}

TimeFrequencyHeatmapRenderer::TimeFrequencyHeatmapRenderer(
  std::shared_ptr<render_gl::TimeFrequencyHeatmapContainer> timeFrequencyHeatmapContainer)
  : m_container{ timeFrequencyHeatmapContainer }
{
    recreateRenderProgram();

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

TimeFrequencyHeatmapRenderer::~TimeFrequencyHeatmapRenderer()
{
    glDeleteVertexArrays(1, &m_quadVao);
    glDeleteBuffers(1, &m_quadVbo);
    glDeleteProgram(m_heatmapShaderProgram);
}

void TimeFrequencyHeatmapRenderer::render(const RenderContext& renderContext)
{
    const auto contSettings = m_container->getSettings();

    const auto lowerLeft = renderContext.ndcToWorld({ -1, -1 });
    const auto upperRight = renderContext.ndcToWorld({ 1, 1 });

    const auto startX =
      m_waterfallDirection == WaterfallDirection::Horizontal ? lowerLeft.x : -upperRight.y;
    const auto endX =
      m_waterfallDirection == WaterfallDirection::Horizontal ? upperRight.x : -lowerLeft.y;

    auto startColumn = startX * contSettings.columnsInOneSecond;
    auto endColumn = endX * contSettings.columnsInOneSecond;

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
        const auto columnWidthSeconds = 1.0f / contSettings.columnsInOneSecond;
        const auto bufferStartSeconds = heatmapBuffer.startColumn * columnWidthSeconds;

        const auto cosA = m_waterfallDirection == WaterfallDirection::Horizontal ? 1.0f : 0.0f;
        const auto sinA = m_waterfallDirection == WaterfallDirection::Horizontal ? 0.0f : -1.0f;

        const glm::mat3 localToWorldMat{ { bufferWidthSeconds, 0, 0 },
                                         { 0, bufferHeightHertz, 0 },
                                         { bufferStartSeconds + bufferWidthSeconds / 2.0f,
                                           contSettings.frequencyOffset + bufferHeightHertz / 2.0f,
                                           1 } };

        // set rendering parameters
        glUseProgram(m_heatmapShaderProgram);

        // set local to clip matrix
        const auto localToClipMat =
          renderContext.camera->getViewProjection() * getRotationMatrix() * localToWorldMat;
        glUniformMatrix3fv(m_localToClipIdx, 1, GL_FALSE, glm::value_ptr(localToClipMat));

        // set local to world matrix
        glUniformMatrix3fv(m_localToWorldIdx, 1, GL_FALSE, glm::value_ptr(localToWorldMat));

        // set OpenGL uniform/SSBO buffer
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, heatmapBuffer.ssbo);

        const auto bufferLeftEdgeOffsetX = heatmapBuffer.startColumn * columnWidthSeconds;
        glUniform2f(m_lowerLeftIdx, bufferLeftEdgeOffsetX, constantHeightOffset);

        glUniform1f(m_columnWidthUnitsIdx, columnWidthSeconds);

        const auto singleColumnElementHeight = 1.0f / contSettings.valuesInOneHertz;
        glUniform1f(m_valueHeightUnitsIdx, singleColumnElementHeight);

        glUniform1ui(m_columnHeightValuesIdx,
                     static_cast<GLuint>(contSettings.columnHeightElementCount));

        glUniform1f(m_minValueIdx, m_scaleMinValue);
        glUniform1f(m_maxValueIdx, m_scaleMaxValue);

        // draw quad
        glBindVertexArray(m_quadVao);
        glBindBuffer(GL_ARRAY_BUFFER, m_quadVbo);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindBuffer(GL_ARRAY_BUFFER, NoBuffer);
        glBindVertexArray(NoBuffer);
    }
}

float TimeFrequencyHeatmapRenderer::getScaleMinValue() const
{
    return m_scaleMinValue;
}

void TimeFrequencyHeatmapRenderer::setScaleMinValue(float newMinValue)
{
    m_scaleMinValue = newMinValue;
}

float TimeFrequencyHeatmapRenderer::getScaleMaxValue() const
{
    return m_scaleMaxValue;
}

void TimeFrequencyHeatmapRenderer::setScaleMaxValue(float newMaxValue)
{
    m_scaleMaxValue = newMaxValue;
}

void TimeFrequencyHeatmapRenderer::resetScaleRange()
{
    m_scaleMinValue = 0;
    m_scaleMaxValue = m_container->getMaxValue();
}

void TimeFrequencyHeatmapRenderer::setMagnitudeColorMode(MagnitudeColorMode magnitudeColorMode)
{
    if (m_magnitudeColorMode != magnitudeColorMode)
    {
        m_magnitudeColorMode = magnitudeColorMode;
        recreateRenderProgram();
    }
}

void TimeFrequencyHeatmapRenderer::setMagnitudeAxisScale(MagnitudeAxisScale magnitudeAxisScale)
{
    if (m_magnitudeAxisScale != magnitudeAxisScale)
    {
        m_magnitudeAxisScale = magnitudeAxisScale;
        recreateRenderProgram();
    }
}

void TimeFrequencyHeatmapRenderer::setFrequencyAxisScale(FrequencyAxisScale frequencyAxisScale)
{
    if (m_frequencyAxisScale != frequencyAxisScale)
    {
        m_frequencyAxisScale = frequencyAxisScale;
        recreateRenderProgram();
    }
}

void TimeFrequencyHeatmapRenderer::setMagnitudeInterpolationMode(
  MagnitudeInterpolationMode magnitudeInterpolationMode)
{
    if (m_magnitudeInterpolationMode != magnitudeInterpolationMode)
    {
        m_magnitudeInterpolationMode = magnitudeInterpolationMode;
        recreateRenderProgram();
    }
}

void TimeFrequencyHeatmapRenderer::setWaterfallDirectionMode(WaterfallDirection waterfallDirection)
{
    if (m_waterfallDirection != waterfallDirection)
    {
        m_waterfallDirection = waterfallDirection;
        // TODO
    }
}

glm::mat3 TimeFrequencyHeatmapRenderer::getRotationMatrix() const
{
    const auto cosA = m_waterfallDirection == WaterfallDirection::Horizontal ? 1.0f : 0.0f;
    const auto sinA = m_waterfallDirection == WaterfallDirection::Horizontal ? 0.0f : -1.0f;
    const auto rotationMatrix = glm::mat3({ cosA, sinA, 0 }, { -sinA, cosA, 0 }, { 0, 0, 1 });
    return rotationMatrix;
}

void TimeFrequencyHeatmapRenderer::recreateRenderProgram()
{
    if (m_heatmapShaderProgram != NoShaderProgram)
    {
        glDeleteProgram(m_heatmapShaderProgram);
    }

    // create shader
    const auto vertexShaderSource =
      utils::File::read(utils::Asset::getPath(HeatmapVertexShaderPath));

    std::vector<std::string> fragmentShaderSources;
    fragmentShaderSources.push_back("#version 430 core\n");

    switch (m_magnitudeColorMode)
    {
        case MagnitudeColorMode::Grayscale:
        {
            fragmentShaderSources.push_back(
              utils::File::read(utils::Asset::getPath(MagnitudeGrayScaleShaderPath)));
            break;
        }
        case MagnitudeColorMode::FiveColorScale:
        {
            fragmentShaderSources.push_back(
              utils::File::read(utils::Asset::getPath(MagnitudeColoredScaleShaderPath)));
            break;
        }
        default:
            throw utils::Exception("Unsupported magnitude color mode: {}",
                                   static_cast<int>(m_magnitudeColorMode));
    }

    switch (m_frequencyAxisScale)
    {
        case FrequencyAxisScale::Linear:
        {
            fragmentShaderSources.push_back(
              utils::File::read(utils::Asset::getPath(FrequencyLinearScaleShaderPath)));
            break;
        }
        case FrequencyAxisScale::Logarithmic:
        {
            fragmentShaderSources.push_back(
              utils::File::read(utils::Asset::getPath(FrequencyLogarithmicScaleShaderPath)));
            break;
        }
        default:
            throw utils::Exception("Unsupported frequency axis scale mode: {}",
                                   static_cast<int>(m_frequencyAxisScale));
    }

    switch (m_magnitudeAxisScale)
    {
        case MagnitudeAxisScale::Linear:
        {
            fragmentShaderSources.push_back(
              utils::File::read(utils::Asset::getPath(MagnitudeScaleLinearShaderPath)));
            break;
        }
        case MagnitudeAxisScale::Logarithmic:
        {
            fragmentShaderSources.push_back(
              utils::File::read(utils::Asset::getPath(MagnitudeScaleLogarithmicShaderPath)));
            break;
        }
        default: break;
    }

    switch (m_magnitudeInterpolationMode)
    {
        case MagnitudeInterpolationMode::Sharp:
        {
            break;
        }
        // case MagnitudeInterpolationMode::Bilinear: break;
        default: break;
    }

    fragmentShaderSources.push_back(
      utils::File::read(utils::Asset::getPath(HeatmapFragmentShaderPath)));

    m_heatmapShaderProgram =
      OpenGlUtils::createShaderProgram({ vertexShaderSource }, fragmentShaderSources);

    // get uniforms indices
    m_localToWorldIdx = glGetUniformLocation(m_heatmapShaderProgram, "localToWorld");
    m_localToClipIdx = glGetUniformLocation(m_heatmapShaderProgram, "localToClip");
    m_lowerLeftIdx = glGetUniformLocation(m_heatmapShaderProgram, "lowerLeft");
    m_columnWidthUnitsIdx = glGetUniformLocation(m_heatmapShaderProgram, "columnWidthUnits");
    m_valueHeightUnitsIdx = glGetUniformLocation(m_heatmapShaderProgram, "valueHeightUnits");
    m_columnHeightValuesIdx = glGetUniformLocation(m_heatmapShaderProgram, "columnHeightValues");
    m_minValueIdx = glGetUniformLocation(m_heatmapShaderProgram, "minValue");
    m_maxValueIdx = glGetUniformLocation(m_heatmapShaderProgram, "maxValue");
}
}
