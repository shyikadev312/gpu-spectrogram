#pragma once

#include <spectr/desktop_app/CameraBoundsController.h>
#include <spectr/render_gl/AxisRenderer.h>
#include <spectr/render_gl/Camera.h>
#include <spectr/render_gl/CheckerGridRenderer.h>
#include <spectr/render_gl/FpsGuard.h>
#include <spectr/render_gl/GraphicsApi.h>
#include <spectr/render_gl/PanTool.h>
#include <spectr/render_gl/RenderContext.h>
#include <spectr/render_gl/ScalableAxisRenderer.h>
#include <spectr/render_gl/TimeFrequencyHeatmapContainer.h>
#include <spectr/render_gl/TimeFrequencyHeatmapRenderer.h>

#include <functional>

namespace spectr::desktop_app
{
class SpectrDesktopApp
{
public:
    friend void keyborardCallbackGlobal(GLFWwindow* window,
                                        int key,
                                        int scancode,
                                        int action,
                                        int mods);

    friend void scrollCallbackGlobal(GLFWwindow* window, double xoffset, double yoffset);

    friend void mouseButtonCallbackGlobal(GLFWwindow* window, int button, int action, int mods);

    friend void cursorPositionCallbackGlobal(GLFWwindow* window, double xpos, double ypos);

    static int main(int argc, char* argv[]);

private:
    int mainImpl(int argc, char* argv[]);

    void initGraphics();

    void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

    void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

    void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

    void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos);

    void onFrameStart();

    void onMainLoopIteration();

    void onBeforeRender();

    void onRender();

    void onAfterRender();

    void onFrameEnd();

    void setScale(float secondToPixelCoeff, float frequencyToPixelCoeff);

private:
    GLFWwindow* m_window = nullptr;

    bool isResetScale = true;
    float m_secondToPixelCoeff = 100;
    float m_frequencyToPixelCoeff = 100; // 20000.0f / 1000.0f;

    std::shared_ptr<render_gl::Camera> m_camera;
    std::shared_ptr<render_gl::RenderContext> m_renderContext;
    std::unique_ptr<render_gl::ScalableAxisRenderer> m_scalableTimeLineRenderer;
    std::unique_ptr<render_gl::ScalableAxisRenderer> m_scalableFrequencyLineRenderer;
    std::unique_ptr<render_gl::CheckerGridRenderer> m_checkerGridRenderer;
    std::shared_ptr<render_gl::TimeFrequencyHeatmapContainer> m_timeFrequencyHeatmapContainer;
    std::shared_ptr<render_gl::TimeFrequencyHeatmapRenderer> m_timeFrequencyHeatmapRenderer;
    std::unique_ptr<render_gl::FpsGuard> m_fpsGuard;
    std::shared_ptr<CameraBoundsController> m_cameraBoundsController;
    bool m_isPanToolEnabled = true;
    std::unique_ptr<render_gl::PanTool> m_panTool;

    std::vector<std::function<void()>> m_onMainLoopActions;
    std::vector<std::function<void(const render_gl::RenderContext&)>> m_onRenderActions;
};
}
