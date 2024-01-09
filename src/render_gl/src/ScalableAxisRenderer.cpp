#include <spectr/render_gl/ScalableAxisRenderer.h>

namespace spectr::render_gl
{
namespace
{
float getAppropriateMarksStep(const glm::vec2 p1, const glm::vec2 p2)
{
    const auto distance = glm::distance(p1, p2);
    const auto distancePowerDouble = std::log10(distance);
    const auto distancePowerInteger = std::lround(distancePowerDouble);
    float step = std::pow(10, distancePowerInteger);
    return step;
}
}

ScalableAxisRenderer::ScalableAxisRenderer(AxisRenderMode axisRenderMode)
  : m_axisMode{ axisRenderMode }
  , m_axis1{ axisRenderMode }
  , m_axis2{ axisRenderMode }
{
    //m_axis2.setIsRenderLabels(false);
}

void ScalableAxisRenderer::render(RenderContext& renderContext)
{
    glm::vec2 pixelPoint1, pixelPoint2;
    if (m_axisMode == AxisRenderMode::Horizontal)
    {
        pixelPoint1 = { 0, 0 };
        pixelPoint2 = { renderContext.viewportSize.x, 0 };
    }
    else
    {
        pixelPoint1 = { 0, 0 };
        pixelPoint2 = { 0, renderContext.viewportSize.y };
    }

    const auto p1 = renderContext.pixelToWorld(pixelPoint1);
    const auto p2 = renderContext.pixelToWorld(pixelPoint2);
    const auto step = getAppropriateMarksStep(p1, p2);
    m_axis1.setMarksStep(step);
    m_axis2.setMarksStep(step / 10.f);

    m_axis1.render(renderContext);
    m_axis2.render(renderContext);
}
}
