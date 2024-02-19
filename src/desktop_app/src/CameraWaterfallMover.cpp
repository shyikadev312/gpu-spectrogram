#include <iostream>
#include <spectr/desktop_app/CameraWaterfallMover.h>
namespace spectr::desktop_app
{
CameraWaterfallMover::CameraWaterfallMover(
  std::shared_ptr<render_gl::RenderContext> renderContext,
  std::shared_ptr<render_gl::TimeFrequencyHeatmapContainer> container)
  : m_renderContext{ renderContext }
  , m_container{ container }
{
    m_lastTime = std::chrono::high_resolution_clock::now();
}

void CameraWaterfallMover::update()
{
    const auto currentTime = std::chrono::high_resolution_clock::now();
    const auto timeDiff = static_cast<std::chrono::duration<float>>(currentTime - m_lastTime);
    m_lastTime = currentTime;

    if (!m_isEnabled)
    {
        return;
    }

    auto camera = m_renderContext->camera;
    const auto maxAvailableTime = m_container->getTimeRange().max;

    const auto cameraPosition = camera->getPositionWorld();
    const auto halfViewportSize = camera->getViewportSize() / 2.0f;

    const auto viewportEdgeCoordinate =
      m_waterfallDirection == render_gl::WaterfallDirection::Horizontal
        ? cameraPosition.x + halfViewportSize.x
        : cameraPosition.y + halfViewportSize.y;

    const auto cameraStickEpsilon = halfViewportSize.x / 10.0f;

    const auto distanceLeft = maxAvailableTime - viewportEdgeCoordinate;
    const auto Epsilon = 1e-5;
    // std::cout << "max time: " << maxAvailableTime << " distanceLEft: " << distanceLeft <<
    // std::endl;
    if (distanceLeft < Epsilon || distanceLeft > cameraStickEpsilon)
    {
        return;
    }
    // std::cout << "timediff: " << timeDiff.count() << std::endl;
    auto newCameraPosition = camera->getPositionWorld(); // glm::vec2(timeDiff.count(), 0);

    if (m_waterfallDirection == render_gl::WaterfallDirection::Horizontal)
    {
        newCameraPosition.x = maxAvailableTime;
    }
    else
    {
        newCameraPosition.y = -maxAvailableTime;
    }

    // std::cout << "oldPosition: " << camera->getPositionWorld().x << std::endl;
    // std::cout << "newPosition: " << newCameraPosition.x << std::endl;
    camera->setPositionWorld(newCameraPosition);
}

void CameraWaterfallMover::setIsEnabled(bool isEnabled)
{
    m_isEnabled = isEnabled;
}

void CameraWaterfallMover::setWaterfallDirection(render_gl::WaterfallDirection waterfallDirection)
{
    m_waterfallDirection = waterfallDirection;
}
}
