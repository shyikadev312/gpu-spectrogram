#pragma once

#include <spectr/render_gl/RenderContext.h>
#include <spectr/render_gl/TimeFrequencyHeatmapRenderer.h>

#include <functional>

namespace spectr::desktop_app
{
struct HeatmapViewSettingsCallbacks
{
    std::function<void(render_gl::MagnitudeColorMode)> onMagnitudeColorModeChanged;
    std::function<void(render_gl::FrequencyAxisScale)> onFrequencyAxisScaleChanged;
    std::function<void(render_gl::MagnitudeAxisScale)> onMagnitudeAxisChanged;
    std::function<void(render_gl::MagnitudeInterpolationMode)> onMagnitudeInterpolationModeChanged;
    std::function<void(render_gl::WaterfallDirection)> onWaterfallDirectionChanged;
    std::function<void(bool)> onShowCursorInfoChanged;
};

class HeatmapViewSettingsWidget
{
public:
    HeatmapViewSettingsWidget(ImFont* font, HeatmapViewSettingsCallbacks callbacks);

    void render(const render_gl::RenderContext& renderContext);

    render_gl::MagnitudeColorMode getMagnitudeColorMode() const;

    void setMagnitudeColorMode(render_gl::MagnitudeColorMode magnitudeColorMode);

    render_gl::FrequencyAxisScale getFrequencyAxisScale() const;

    void setFrequencyAxisScale(render_gl::FrequencyAxisScale frequencyAxisScale);

    render_gl::MagnitudeAxisScale getMagnitudeAxisScale() const;

    void setMagnitudeAxisScale(render_gl::MagnitudeAxisScale magnitudeAxisScale);

    render_gl::MagnitudeInterpolationMode getMagnitudeInterpolationMode() const;

    void setMagnitudeInterpolationMode(
      render_gl::MagnitudeInterpolationMode magnitudeInterpolationMode);

    render_gl::WaterfallDirection getWaterfallDirection() const;

    void setWaterfallDirection(render_gl::WaterfallDirection waterfallDirection);

    bool getShowCursorInfo() const;

    void setShowCursorInfo(bool isShowCursorInfo);

private:
    ImFont* m_font = nullptr;
    HeatmapViewSettingsCallbacks m_callbacks;
    render_gl::MagnitudeColorMode m_magnitudeColorMode = render_gl::MagnitudeColorMode::Grayscale;
    render_gl::FrequencyAxisScale m_frequencyAxisScale = render_gl::FrequencyAxisScale::Linear;
    render_gl::MagnitudeAxisScale m_magnitudeAxisScale = render_gl::MagnitudeAxisScale::Linear;
    render_gl::MagnitudeInterpolationMode m_magnitudeInterpolationMode =
      render_gl::MagnitudeInterpolationMode::Sharp;
    render_gl::WaterfallDirection m_waterfallDirection = render_gl::WaterfallDirection::Horizontal;
    bool m_isShowCursorInfo = true;
};
}
