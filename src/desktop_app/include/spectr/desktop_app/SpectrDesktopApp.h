#pragma once

#include <spectr/desktop_app/DesktopAppSettings.h>
#include <spectr/desktop_app/Input.h>
#include <spectr/desktop_app/RtsaWindow.h>
#include <spectr/desktop_app/WaterfallWindow.h>
#include <spectr/render_gl/FpsGuard.h>
#include <spectr/render_gl/GlfwUtils.h>
#include <spectr/render_gl/RtsaContainer.h>
#include <spectr/render_gl/TimeFrequencyHeatmapContainer.h>

#include <functional>
#include <memory>
#include <vector>

namespace spectr::desktop_app
{
/**
 * @brief Main entry point for Spectr desktop UI application.
 * @details This class parses command line arguments, initializes graphics API, creates GLFW window
 * on screen, creates spectrogram waterfall view and spectrogram density view (real-time
 * spectrum analyzer RTSA), initializes OpenCL API, loads audio file and launches FFT calculations.
 */
class SpectrDesktopApp
{
public:
    static int main(int argc, char* argv[]);

private:
    int mainImpl(int argc, char* argv[]);

    void initGraphics();

    void initFftCalculator(const DesktopAppSettings& settings);

private:
    std::unique_ptr<render_gl::GlfwWrapper> m_glfwApi;
    render_gl::GlfwWindowWrapper m_window;
    std::shared_ptr<Input> m_input;
    std::shared_ptr<WaterfallWindow> m_waterfallWindow;
    std::shared_ptr<RtsaWindow> m_rtsaWindow;
    std::shared_ptr<Window> m_currentWindow = nullptr;
    std::shared_ptr<render_gl::TimeFrequencyHeatmapContainer> m_timeFrequencyHeatmapContainer;
    std::shared_ptr<render_gl::RtsaContainer> m_rtsaHeatmapContainer;
    std::unique_ptr<render_gl::FpsGuard> m_fpsGuard;
    std::vector<std::function<void()>> m_onMainLoopActions;
};
}
