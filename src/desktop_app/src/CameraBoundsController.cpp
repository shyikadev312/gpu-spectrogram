#include <spectr/desktop_app/CameraBoundsController.h>

namespace spectr::desktop_app
{
CameraBoundsController::CameraBoundsController(
  std::shared_ptr<render_gl::RenderContext> renderContext)
  : m_renderContext{ renderContext }
{
    //
}

void CameraBoundsController::update()
{
    auto camera = m_renderContext->camera;

    const auto boundsSize = m_upperRight - m_lowerLeft;
    const auto boundsSizeX = boundsSize.x;
    const auto boundsSizeY = boundsSize.y;

    // calculate and set camera min scale
    const auto viewportSize = m_renderContext->viewportSize;
    const auto minScaleX = boundsSizeX / viewportSize.x;
    const auto minScaleY = 0.0f;

    const auto boundedScaleX = std::max(camera->getScaleX(), minScaleX);
    const auto boundedScaleY = std::max(camera->getScaleY(), minScaleY);

    camera->setScale({ boundedScaleX, boundedScaleY });

    // calculate camera min/max position
}

void CameraBoundsController::setBounds(const glm::vec2& lowerLeft, const glm::vec2& upperRight)
{
    m_lowerLeft = lowerLeft;
    m_upperRight = upperRight;
}

float CameraBoundsController::getMinX() const
{
    return m_lowerLeft.x;
}

float CameraBoundsController::getMaxX() const
{
    return m_upperRight.x;
}

float CameraBoundsController::getMinY() const
{
    return m_lowerLeft.y;
}

float CameraBoundsController::getMaxY() const
{
    return m_upperRight.y;
}
}
