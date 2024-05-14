#pragma once

#include <spectr/desktop_app/CameraBoundsController.h>
#include <spectr/desktop_app/HeatmapCursorInfo.h>
#include <spectr/desktop_app/HeatmapViewSettingsWidget.h>
#include <spectr/desktop_app/Input.h>
#include <spectr/desktop_app/PanTool.h>
#include <spectr/desktop_app/Window.h>
#include <spectr/render_gl/AxisRenderer.h>
#include <spectr/render_gl/Camera.h>
#include <spectr/render_gl/CheckerGridRenderer.h>
#include <spectr/render_gl/GraphicsApi.h>
#include <spectr/render_gl/TimeFrequencyHeatmapContainer.h>
#include <spectr/render_gl/TimeFrequencyHeatmapRenderer.h>

namespace spectr::desktop_app
{
class WaterfallWindow : public Window
{
public:
    WaterfallWindow(std::shared_ptr<Input> input,
                    std::shared_ptr<render_gl::TimeFrequencyHeatmapContainer> container,
                    size_t offset);

    void onMainLoopUpdate() override;

    void onBeforeRender() override;

    void onRender() override;

    void onAfterRender() override;

private:
    void recreateAxis();

    void setScale(float secondToPixelCoeff, float frequencyToPixelCoeff);

private:
    std::shared_ptr<Input> m_input;
    std::shared_ptr<render_gl::TimeFrequencyHeatmapContainer> m_timeFrequencyHeatmapContainer;
    bool isResetScale = true;
    std::shared_ptr<render_gl::Camera> m_camera;
    std::shared_ptr<render_gl::RenderContext> m_renderContext;
    std::shared_ptr<render_gl::CheckerGridRenderer> m_checkerGridRenderer;
    std::shared_ptr<render_gl::TimeFrequencyHeatmapRenderer> m_heatmapRenderer;
    std::shared_ptr<CameraBoundsController> m_cameraBoundsController;
    std::unique_ptr<PanTool> m_panTool;
    std::shared_ptr<HeatmapCursorInfo> m_cursorInfoWidget;
    std::shared_ptr<HeatmapViewSettingsWidget> m_heatmapViewSettingsWidget;
    ImFont* m_axisFont = nullptr;
    std::vector<std::shared_ptr<render_gl::AxisRenderer>> m_axes;
    size_t m_frequencyOffset;
};
}
