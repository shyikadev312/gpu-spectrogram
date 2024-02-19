#pragma once

#include <spectr/desktop_app/HeatmapCursorInfo.h>
#include <spectr/desktop_app/Input.h>
#include <spectr/desktop_app/PanTool.h>
#include <spectr/desktop_app/RtsaViewSettingsWidget.h>
#include <spectr/desktop_app/Window.h>
#include <spectr/render_gl/AxisRenderer.h>
#include <spectr/render_gl/Camera.h>
#include <spectr/render_gl/CheckerGridRenderer.h>
#include <spectr/render_gl/GraphicsApi.h>
#include <spectr/render_gl/RenderContext.h>
#include <spectr/render_gl/RtsaContainer.h>
#include <spectr/render_gl/RtsaRenderer.h>

namespace spectr::desktop_app
{
class RtsaWindow : public Window
{
public:
    RtsaWindow(std::shared_ptr<Input> input, std::shared_ptr<render_gl::RtsaContainer> container);

    void onMainLoopUpdate() override;

    void onBeforeRender() override;

    void onRender() override;

    void onAfterRender() override;

private:
    void recreateAxis();

private:
    std::shared_ptr<Input> m_input;
    std::shared_ptr<render_gl::RtsaContainer> m_container;
    std::shared_ptr<render_gl::CheckerGridRenderer> m_checkerGridRenderer;
    std::shared_ptr<render_gl::RtsaRenderer> m_rtsaRenderer;
    std::unique_ptr<PanTool> m_panTool;
    std::shared_ptr<HeatmapCursorInfo> m_cursorInfoWidget;
    std::shared_ptr<RtsaViewSettingsWidget> m_rtsaViewSettingsWidget;
    std::shared_ptr<render_gl::Camera> m_camera;
    std::shared_ptr<render_gl::RenderContext> m_renderContext;
    ImFont* m_axisFont = nullptr;
    std::vector<std::shared_ptr<render_gl::AxisRenderer>> m_axes;
};
}
