#include <spectr/render_gl/AxisRenderer.h>

#include <spectr/render_gl/ImguiUtils.h>

#include <spectr/utils/Assert.h>
#include <spectr/utils/Asset.h>
#include <spectr/utils/Exception.h>
#include <spectr/utils/File.h>

#include <fmt/format.h>
#include <glm/gtc/type_ptr.hpp>

#include <cmath>
#include <vector>

namespace spectr::render_gl
{
namespace
{
const auto LineVertexShaderPath = "shaders/line/Vertex.glsl";
const auto LineFragmentShaderPath = "shaders/line/Fragment.glsl";

// Quad of size 1x1 with center in point (0;0)
const std::vector<float> UnitQuadVertices{
    -0.5f, -0.5f, 0.0f, // left-down
    0.5f,  -0.5f, 0.0f, // right-down
    0.5f,  0.5f,  0.0f, // right-top

    -0.5f, -0.5f, 0.0f, // left-down
    0.5f,  0.5f,  0.0f, // right-top
    -0.5f, 0.5f,  0.0f, // left-top
};

float getAppropriateMarksStep(float visibleAxisLength)
{
    const auto detailsCoeff = 0.7f;
    const auto distancePower = std::log10(visibleAxisLength) - detailsCoeff;
    const auto distancePowerRounded = std::round(distancePower);
    const auto step = std::pow(10.0f, distancePowerRounded);
    return step;
}

float getAppropriateMarksStep(float visibleAxisLength, int detailOrder)
{
    const auto markStepBase = getAppropriateMarksStep(visibleAxisLength);
    const auto markStep = markStepBase / std::pow(10.0f, static_cast<float>(detailOrder));
    return markStep;
}

/**
 * @brief Check if axis is horizontal.
 * @return True if axis direction is horizontal; false if axis direction is vertical; Exception
 * if axis direction is neither horizontal nor vertical.
 */
bool calculateIsHorizontal(const glm::vec2& direction)
{
    ASSERT(std::abs(glm::length(direction) - 1.0f) < 1e-5);
    constexpr float Epsilon = 1e-5f;
    const auto dot = std::abs(glm::dot(direction, glm::vec2(1, 0)));
    if (std::abs(1.0f - dot) < Epsilon)
    {
        return true;
    }
    if (dot < Epsilon)
    {
        return false;
    }
    throw std::runtime_error("Axis direction is neither horizontal nor vertical.");
}

std::string toString(AxisDockPosition axisDockPosition)
{
    switch (axisDockPosition)
    {
        case spectr::render_gl::AxisDockPosition::Left: return "Left";
        case spectr::render_gl::AxisDockPosition::Right: return "Right";
        case spectr::render_gl::AxisDockPosition::Top: return "Top";
        case spectr::render_gl::AxisDockPosition::Bottom: return "Bottom";
        case spectr::render_gl::AxisDockPosition::Center: return "Center";
        default: throw utils::Exception("Unrecognized AxisDockPosition.");
    }
}

std::string toStringDirection(bool isHorizontal)
{
    return isHorizontal ? "Horizontal" : "Vertical";
}
}

AxisRenderer::AxisRenderer(const glm::vec2& axisDirection,
                           AxisDockPosition axisDockPosition,
                           ImFont* font,
                           int detailOrder)
  : m_axisDirection{ axisDirection }
  , m_axisDockPosition{ axisDockPosition }
  , m_isHorizontal{ calculateIsHorizontal(axisDirection) }
  , m_detailOrder{ detailOrder }
  , m_font{ font }
{
    const auto vertexShaderSource = utils::File::read(utils::Asset::getPath(LineVertexShaderPath));
    const auto fragmentShaderSource =
      utils::File::read(utils::Asset::getPath(LineFragmentShaderPath));

    m_lineShaderProgram =
      OpenGlUtils::createShaderProgram(vertexShaderSource, fragmentShaderSource);

    glGenBuffers(1, &m_lineVbo);
    glGenVertexArrays(1, &m_lineVao);

    glBindVertexArray(m_lineVao);

    glBindBuffer(GL_ARRAY_BUFFER, m_lineVbo);
    glBufferData(GL_ARRAY_BUFFER,
                 UnitQuadVertices.size() * sizeof(float),
                 UnitQuadVertices.data(),
                 GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, NoBuffer);
    glBindVertexArray(NoBuffer);

    m_lineColorId = glGetUniformLocation(m_lineShaderProgram, "lineColor");
    m_ctmId = glGetUniformLocation(m_lineShaderProgram, "ctm");
}

AxisRenderer::~AxisRenderer()
{
    glDeleteVertexArrays(1, &m_lineVao);
    glDeleteBuffers(1, &m_lineVbo);
    glDeleteProgram(m_lineShaderProgram);
}

void AxisRenderer::render(const RenderContext& renderContext)
{
    if (!m_isEnabled)
    {
        return;
    }

    const auto [startVisibleAxisWorldPoint, endVisibleAxisWorldPoint] =
      getVisibleAxisWorldRange(renderContext);

    const auto startVisibleAxisCoord =
      m_isHorizontal ? startVisibleAxisWorldPoint.x : startVisibleAxisWorldPoint.y;

    const auto endVisibleAxisCoord =
      m_isHorizontal ? endVisibleAxisWorldPoint.x : endVisibleAxisWorldPoint.y;

    const auto visibleAxisLength = endVisibleAxisCoord - startVisibleAxisCoord;
    const auto markStep = getAppropriateMarksStep(visibleAxisLength, m_detailOrder);

    const auto startMarksCount = startVisibleAxisCoord / markStep;
    const auto endMarksCount = endVisibleAxisCoord / markStep;

    const auto startMarksCountRounded = std::floor(startMarksCount);
    const auto endMarksCountRounded = std::ceil(endMarksCount);

    const auto firstVisisbleMarkCoordinate = startMarksCountRounded * markStep;
    const auto marksCount = static_cast<size_t>(endMarksCount - startMarksCount + 1);

    for (size_t i = 0; i <= marksCount; ++i)
    {
        const auto worldCoordinate = firstVisisbleMarkCoordinate + markStep * i;
        const auto currentAxisCoordinate = getWorldToAxis(worldCoordinate);

        // const auto currentAxisCoordinate = firstVisisbleMarkAxisCoordinate + markStep * i;
        // const auto worldCoordinate = getAxisToWorld(currentAxisCoordinate);

        auto digitsCount = static_cast<int>(std::log10(markStep));
        digitsCount = digitsCount > 0 ? 0 : -digitsCount;
        const auto formatString = fmt::format("{{:.{}f}}", digitsCount);
        const auto valueText = fmt::format(fmt::runtime(formatString), currentAxisCoordinate);
        const auto labelText = fmt::format("{}{}", valueText, m_suffix);
        renderMark(worldCoordinate, labelText, renderContext);
    }
}

void AxisRenderer::renderMark(float worldCoordinate,
                              const std::string& text,
                              const RenderContext& renderContext)
{
    const auto markWorldPoint = getWorldPointFromAxisCoordinate(worldCoordinate, renderContext);
    const auto markPixelCoord = renderContext.worldToPixel(markWorldPoint);

    const auto lineStartPixelPoint = markPixelCoord;

    glm::ivec2 markEndOffset{};
    if (isHorizontal())
    {
        if (m_axisDockPosition == AxisDockPosition::Top)
        {
            markEndOffset = { 0, -m_markHeight };
        }
        else if (m_axisDockPosition == AxisDockPosition::Bottom)
        {
            markEndOffset = { 0, m_markHeight };
        }
    }
    else
    {
        if (m_axisDockPosition == AxisDockPosition::Left)
        {
            markEndOffset = { m_markHeight, 0 };
        }
        else if (m_axisDockPosition == AxisDockPosition::Right)
        {
            markEndOffset = { -m_markHeight, 0 };
        }
    }

    const auto lineEndPixelPoint = lineStartPixelPoint + markEndOffset;

    if (m_isRenderLabels)
    {
        const auto title = toStringDirection(isHorizontal()) + toString(m_axisDockPosition) +
                           std::to_string(worldCoordinate);
        const auto lineCenter = glm::vec2(lineStartPixelPoint + lineEndPixelPoint) / 2.0f;
        renderLabel(title, text, lineCenter);
    }

    if (m_isRenderMarks)
    {
        renderLine(lineStartPixelPoint, lineEndPixelPoint, m_markWidth, m_markColor, renderContext);
    }
}

void AxisRenderer::renderLine(const glm::vec2& lineStartPixel,
                              const glm::vec2& lineEndPixel,
                              const float lineWidthPixel,
                              const Color& lineColor,
                              const RenderContext& renderContext)
{
    const auto lineWidth = m_markWidth;
    const auto lineHeight = glm::distance(lineStartPixel, lineEndPixel);

    const auto scaleMatrix = glm::mat3({ lineWidth, 0, 0 }, { 0, lineHeight, 0 }, { 0, 0, 1 });

    const auto lineMiddle = (lineStartPixel + lineEndPixel) / 2.0f;
    const auto translation = lineMiddle;
    const auto translationMatrix =
      glm::mat3({ 1, 0, 0 }, { 0, 1, 0 }, { translation.x, translation.y, 1 });

    const auto lineDirection = glm::normalize(lineEndPixel - lineStartPixel);
    const auto cosA = glm::dot(glm::vec2(0, 1), lineDirection);
    const auto sinA = glm::sqrt(1 - std::pow(cosA, 2));
    const auto rotationMatrix = glm::mat3({ cosA, sinA, 0 }, { -sinA, cosA, 0 }, { 0, 0, 1 });

    const auto pixelSpaceTransformMatrix = translationMatrix * rotationMatrix * scaleMatrix;

    const auto ndcToPixelScale = glm::mat3(glm::vec3(renderContext.viewportSize.x / 2.0f, 0, 0),
                                           glm::vec3(0, renderContext.viewportSize.y / 2.0f, 0),
                                           glm::vec3(0, 0, 1));

    const auto ndcToPixelTranslate = glm::mat3(
      glm::vec3(1, 0, 0),
      glm::vec3(0, 1, 0),
      glm::vec3(renderContext.viewportSize.x / 2.0f, renderContext.viewportSize.y / 2.0f, 1));

    const auto ndcToPixelMatrix = ndcToPixelTranslate * ndcToPixelScale;
    const auto pixelToNdcMatrix = glm::inverse(ndcToPixelMatrix);
    const auto ctm = pixelToNdcMatrix * pixelSpaceTransformMatrix;

    glUseProgram(m_lineShaderProgram);
    glUniform4fv(m_lineColorId, 1, glm::value_ptr(lineColor));
    glUniformMatrix3fv(m_ctmId, 1, GL_FALSE, glm::value_ptr(ctm));
    glBindVertexArray(m_lineVao);
    glBindBuffer(GL_ARRAY_BUFFER, m_lineVbo);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindBuffer(GL_ARRAY_BUFFER, NoBuffer);
    glBindVertexArray(NoBuffer);
}

void AxisRenderer::renderLabel(const std::string& title,
                               const std::string& text,
                               const glm::ivec2& pixelCoords)
{
    const auto windowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
                             ImGuiWindowFlags_NoSavedSettings |
                             ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav |
                             ImGuiWindowFlags_NoMove;
    const auto viewport = ImGui::GetMainViewport();
    const auto viewportSize = viewport->WorkSize;
    const auto windowPosition =
      ImVec2{ static_cast<float>(pixelCoords.x), viewportSize.y - pixelCoords.y };

    ImVec2 windowPivotPoint{ 0.5f, 0.5f };
    switch (m_axisDockPosition)
    {
        case AxisDockPosition::Left:
        {
            windowPivotPoint.x = 0;
            break;
        }
        case AxisDockPosition::Right:
        {
            windowPivotPoint.x = 1;
            break;
        }
        case AxisDockPosition::Bottom:
        {
            windowPivotPoint.y = 1;
            break;
        }
        case AxisDockPosition::Top:
        {
            windowPivotPoint.y = 0;
            break;
        }
    }

    ImGui::PushFont(m_font);
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));

    ImGui::SetNextWindowPos(windowPosition, ImGuiCond_Always, windowPivotPoint);
    ImGui::SetNextWindowBgAlpha(0.8f);
    ImGui::Begin(title.c_str(), nullptr, windowFlags);
    ImGui::Text(text.c_str());
    ImGui::End();

    ImGui::PopStyleColor();
    ImGui::PopFont();
}

std::pair<glm::vec2, glm::vec2> AxisRenderer::getVisibleAxisPixelRange(
  const RenderContext& renderContext) const
{
    glm::ivec2 startVisibleAxisPixelCoord;
    glm::ivec2 endVisibleAxisPixelCoord;
    if (isHorizontal())
    {
        startVisibleAxisPixelCoord = { 0, 0 };
        endVisibleAxisPixelCoord = { renderContext.viewportSize.x, 0 };

        if (m_axisDockPosition == AxisDockPosition::Top)
        {
            startVisibleAxisPixelCoord.y = renderContext.viewportSize.y;
            endVisibleAxisPixelCoord.y = renderContext.viewportSize.y;
        }

        if (!isSameDirection())
        {
            std::swap(startVisibleAxisPixelCoord, endVisibleAxisPixelCoord);
        }
    }
    else
    {
        startVisibleAxisPixelCoord = { 0, 0 };
        endVisibleAxisPixelCoord = { 0, renderContext.viewportSize.y };

        if (m_axisDockPosition == AxisDockPosition::Right)
        {
            startVisibleAxisPixelCoord.x = renderContext.viewportSize.x;
            endVisibleAxisPixelCoord.x = renderContext.viewportSize.x;
        }

        if (!isSameDirection())
        {
            std::swap(startVisibleAxisPixelCoord, endVisibleAxisPixelCoord);
        }
    }

    return { startVisibleAxisPixelCoord, endVisibleAxisPixelCoord };
}

std::pair<glm::vec2, glm::vec2> AxisRenderer::getVisibleAxisWorldRange(
  const RenderContext& renderContext) const
{
    const auto [startPixelPoint, endPixelPoint] = getVisibleAxisPixelRange(renderContext);

    auto startVisibleAxisWorldPoint = renderContext.pixelToWorld(startPixelPoint);
    auto endVisibleAxisWorldPoint = renderContext.pixelToWorld(endPixelPoint);

    if (!isSameDirection())
    {
        std::swap(startVisibleAxisWorldPoint, endVisibleAxisWorldPoint);
    }

    return { startVisibleAxisWorldPoint, endVisibleAxisWorldPoint };
}

Range AxisRenderer::getVisibleAxisCoordinatesRange(const RenderContext& renderContext) const
{
    const auto [startVisibleAxisWorldPoint, endVisibleAxisWorldPoint] =
      getVisibleAxisWorldRange(renderContext);

    const auto startVisibleAxisWorldCoord =
      m_isHorizontal ? startVisibleAxisWorldPoint.x : startVisibleAxisWorldPoint.y;

    const auto endVisibleAxisWorldCoord =
      m_isHorizontal ? endVisibleAxisWorldPoint.x : endVisibleAxisWorldPoint.y;

    const auto startVisibleAxisCoord = getWorldToAxis(startVisibleAxisWorldCoord);
    const auto endVisibleAxisCoord = getWorldToAxis(endVisibleAxisWorldCoord);

    return { startVisibleAxisCoord, endVisibleAxisCoord };
}

glm::vec2 AxisRenderer::getWorldPointFromAxisCoordinate(float worldCoordinate,
                                                        const RenderContext& renderContext) const
{
    // const auto worldCoordinate = getAxisToWorld(axisCoordinate);
    const auto [startPixelPoint, _] = getVisibleAxisPixelRange(renderContext);
    const auto referenceWorldPoint = renderContext.pixelToWorld(startPixelPoint);
    const auto referenceWorldCoordinate =
      isHorizontal() ? referenceWorldPoint.y : referenceWorldPoint.x;

    if (isHorizontal())
    {
        return { worldCoordinate, referenceWorldCoordinate };
    }
    else
    {
        return { referenceWorldCoordinate, worldCoordinate };
    }
}

float AxisRenderer::getWorldToAxis(float worldCoordinate) const
{
    if (!isSameDirection())
    {
        worldCoordinate *= -1;
    }

    return worldCoordinate;
}

float AxisRenderer::getAxisToWorld(float axisCoordinate) const
{
    return axisCoordinate;
}

bool AxisRenderer::isHorizontal() const
{
    return m_isHorizontal;
}

bool AxisRenderer::isSameDirection() const
{
    return m_axisDirection.x > 0.5f || m_axisDirection.y > 0.5f;
}

bool AxisRenderer::getIsEnabled() const
{
    return m_isEnabled;
}

void AxisRenderer::setIsEnabled(bool isEnabled)
{
    m_isEnabled = isEnabled;
}

bool AxisRenderer::getIsRenderLabels() const
{
    return m_isRenderLabels;
}

void AxisRenderer::setIsRenderLabels(bool isRenderLabels)
{
    m_isRenderLabels = isRenderLabels;
}

bool AxisRenderer::getIsRenderMarks() const
{
    return m_isRenderMarks;
}

void AxisRenderer::setIsRenderMarks(bool isRenderMarks)
{
    m_isRenderMarks = isRenderMarks;
}

float AxisRenderer::getAxisMarkStep(const RenderContext& renderContext) const
{
    const auto [startVisibleAxisCoord, endVisibleAxisCoord] =
      getVisibleAxisCoordinatesRange(renderContext);
    const auto visibleAxisLength = endVisibleAxisCoord - startVisibleAxisCoord;
    const auto markStep = getAppropriateMarksStep(visibleAxisLength, m_detailOrder);
    return markStep;
}

float AxisRenderer::getMarkHeight() const
{
    return m_markHeight;
}

void AxisRenderer::setMarkHeight(float markHeightPixels)
{
    m_markHeight = markHeightPixels;
}

void AxisRenderer::setLabelSuffix(std::string suffix)
{
    m_suffix = std::move(suffix);
}

LogarithmicAxisRenderer::LogarithmicAxisRenderer(Range valuesRange,
                                                 const glm::vec2& axisDirection,
                                                 AxisDockPosition axisDockPosition,
                                                 ImFont* font,
                                                 int detailOrder)
  : AxisRenderer(axisDirection, axisDockPosition, font, detailOrder)
  , m_valuesRange{ valuesRange }
{
}

float LogarithmicAxisRenderer::getWorldToAxis(float worldCoordinate) const
{
    const auto minLog = std::log10(m_valuesRange.min + 1.0f);
    const auto maxLog = std::log10(m_valuesRange.max + 1.0f);
    const auto coeff = m_valuesRange.max / maxLog;

    const auto mappedY = worldCoordinate / coeff;
    const auto axisCoordinate = std::pow(10.0f, mappedY) - 1.0f; // axisCoordinate == frequencyValue
    return axisCoordinate;
}

float LogarithmicAxisRenderer::getAxisToWorld(float axisCoordinate) const
{
    const auto minLog = std::log10(m_valuesRange.min + 1.0f);
    const auto maxLog = std::log10(m_valuesRange.max + 1.0f);
    const auto coeff = m_valuesRange.max / maxLog;
    const auto worldCoordinate = std::log10(axisCoordinate + 1.0f) * coeff;
    return worldCoordinate;
}
}
