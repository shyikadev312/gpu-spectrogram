#pragma once

#include <spectr/desktop_app/CameraBoundsController.h>
#include <spectr/desktop_app/HeatmapCursorInfo.h>
#include <spectr/desktop_app/Input.h>
#include <spectr/desktop_app/PanTool.h>
#include <spectr/desktop_app/Window.h>
#include <spectr/render_gl/AxisRenderer.h>
#include <spectr/render_gl/Camera.h>
#include <spectr/render_gl/CheckerGridRenderer.h>
#include <spectr/render_gl/GraphicsApi.h>
#include <spectr/render_gl/TimeFrequencyHeatmapContainer.h>
#include <spectr/render_gl/TimeFrequencyHeatmapRenderer.h>
#include <spectr/render_gl/RtsaContainer.h>
#include <spectr/render_gl/RtsaRenderer.h>

namespace spectr::desktop_app {
    class SplitWindow : public Window {
    public:
        SplitWindow(std::shared_ptr<Input> input,
                    std::shared_ptr<render_gl::TimeFrequencyHeatmapContainer> container,
                    std::shared_ptr<render_gl::RtsaContainer> rtsaContainer);

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
        ImFont* m_axisFont = nullptr;
        std::vector<std::shared_ptr<render_gl::AxisRenderer>> m_axes;
        std::shared_ptr<render_gl::Camera> m_rtsaCamera;
        std::shared_ptr<render_gl::RenderContext> m_rtsaRenderContext;
        std::shared_ptr<render_gl::RtsaContainer> m_rtsaContainer;
        std::shared_ptr<render_gl::RtsaRenderer> m_rtsaRenderer;
    };
}
