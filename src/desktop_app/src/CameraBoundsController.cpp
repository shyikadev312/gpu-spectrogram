#include <spectr/desktop_app/CameraBoundsController.h>

#include <algorithm>

namespace spectr::desktop_app
{
namespace
{
constexpr glm::vec2 MinSize{ 20, 20 };

glm::vec2 getBoundsSize(const glm::vec2& lowerLeft,
                        const glm::vec2 upperRight,
                        const glm::vec2& minSize)
{
    auto size = upperRight - lowerLeft;
    size.x = std::max(size.x, MinSize.x);
    size.y = std::max(size.y, MinSize.y);
    return size;
}
}

CameraBoundsController::CameraBoundsController(
  std::shared_ptr<render_gl::RenderContext> renderContext)
  : m_renderContext{ renderContext }
{
}

void CameraBoundsController::update()
{
    auto camera = m_renderContext->camera;
    const auto boundsSize = getBoundsSize(m_lowerLeft, m_upperRight, MinSize);

    // calculate and set camera min scale
    const auto viewportSize = m_renderContext->viewportSize;
    const auto minScaleX = 2.0f / boundsSize.x;
    const auto minScaleY = 2.0f / boundsSize.y;

    const auto boundedScaleX = std::max(camera->getScaleX(), minScaleX);
    const auto boundedScaleY = std::max(camera->getScaleY(), minScaleY);

    camera->setScale({ boundedScaleX, boundedScaleY });

    // calculate camera min/max position
    const auto cameraViewportSize = 2.0f / camera->getScale();
    const auto halfCameraViewportSize = cameraViewportSize / 2.0f;

    const auto cameraMin = m_lowerLeft + halfCameraViewportSize;
    const auto cameraMax = m_upperRight - halfCameraViewportSize;

    auto cameraPosition = camera->getPositionWorld();
    cameraPosition.x = std::max(cameraPosition.x, cameraMin.x);
    cameraPosition.y = std::max(cameraPosition.y, cameraMin.y);
    cameraPosition.x = std::min(cameraPosition.x, cameraMax.x);
    cameraPosition.y = std::min(cameraPosition.y, cameraMax.y);
    camera->setPositionWorld(cameraPosition);
}

void CameraBoundsController::setBounds(const glm::vec2& lowerLeft, const glm::vec2& upperRight)
{
    m_lowerLeft = lowerLeft;
    m_upperRight = upperRight;
}

void CameraBoundsController::setBoundsX(const glm::vec2& boundsX)
{
    m_lowerLeft.x = boundsX.x;
    m_upperRight.x = boundsX.y;
}

void CameraBoundsController::setBoundsY(const glm::vec2& boundsY)
{
    m_lowerLeft.y = boundsY.x;
    m_upperRight.y = boundsY.y;
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
