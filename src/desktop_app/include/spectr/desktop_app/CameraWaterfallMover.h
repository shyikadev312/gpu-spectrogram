#pragma once

#include <spectr/render_gl/RenderContext.h>
#include <spectr/render_gl/TimeFrequencyHeatmapContainer.h>
#include <spectr/render_gl/TimeFrequencyHeatmapRenderer.h>

#include <chrono>

namespace spectr::desktop_app
{
class CameraWaterfallMover
{
public:
    CameraWaterfallMover(std::shared_ptr<render_gl::RenderContext> renderContext,
                         std::shared_ptr<render_gl::TimeFrequencyHeatmapContainer> container);

    void update();

    void setIsEnabled(bool isEnabled);

    void setWaterfallDirection(render_gl::WaterfallDirection waterfallDirection);

private:
    bool m_isEnabled = true;
    std::shared_ptr<render_gl::RenderContext> m_renderContext;
    std::shared_ptr<render_gl::TimeFrequencyHeatmapContainer> m_container;
    std::chrono::high_resolution_clock::time_point m_lastTime;
    render_gl::WaterfallDirection m_waterfallDirection = render_gl::WaterfallDirection::Horizontal;
};
}
