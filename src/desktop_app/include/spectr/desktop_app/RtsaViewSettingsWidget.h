#pragma once

#include <spectr/render_gl/TimeFrequencyHeatmapRenderer.h>

#include <functional>

namespace spectr::desktop_app
{
struct RtsaViewSettingsWidgetSettings
{
    std::function<void(render_gl::FrequencyAxisScale)> onFrequencyAxisScaleChanged;
};

class RtsaViewSettingsWidget
{
public:
    RtsaViewSettingsWidget(ImFont* font, RtsaViewSettingsWidgetSettings settings);

    void render(const render_gl::RenderContext& renderContext);

    render_gl::FrequencyAxisScale getFrequencyAxisScale() const;

    void setFrequencyAxisScale(render_gl::FrequencyAxisScale frequencyAxisScale);

private:
    ImFont* m_font = nullptr;
    RtsaViewSettingsWidgetSettings m_settings;
    render_gl::FrequencyAxisScale m_frequencyAxisScale = render_gl::FrequencyAxisScale::Linear;
};
}
