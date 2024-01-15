#include <spectr/desktop_app/SpectrDesktopApp.h>

#include <spectr/audio_loader/AudioDataGenerator.h>
#include <spectr/audio_loader/AudioLoader.h>
#include <spectr/calc_opencl/FftCooleyTukeyRadix2.h>
#include <spectr/calc_opencl/OpenclManager.h>
#include <spectr/desktop_app/AudioFileTimeFrequencyWorker.h>
#include <spectr/desktop_app/MinMaxWidget.h>
#include <spectr/desktop_app/MockTimeFrequencyWorker.h>
#include <spectr/render_gl/ImguiUtils.h>
#include <spectr/utils/Assert.h>
#include <spectr/utils/Asset.h>
#include <spectr/utils/Exception.h>

#include <spdlog/spdlog.h>

#include <cstdlib>
#include <filesystem>
#include <iostream>

namespace spectr::desktop_app
{
namespace
{
constexpr auto DefaultWindowWidth = 1920;
constexpr auto DefaultWindowHeight = 1080;
const auto WindowTitle = "Spectrogram renderer";

SpectrDesktopApp* getApp(GLFWwindow* window)
{
    return reinterpret_cast<SpectrDesktopApp*>(glfwGetWindowUserPointer(window));
}
}

void keyborardCallbackGlobal(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    auto spectrApp = getApp(window);
    spectrApp->keyboardCallback(window, key, scancode, action, mods);
}

void scrollCallbackGlobal(GLFWwindow* window, double xoffset, double yoffset)
{
    auto spectrApp = getApp(window);
    spectrApp->scrollCallback(window, xoffset, yoffset);
}

void mouseButtonCallbackGlobal(GLFWwindow* window, int button, int action, int mods)
{
    auto spectrApp = getApp(window);
    spectrApp->mouseButtonCallback(window, button, action, mods);
}

void cursorPositionCallbackGlobal(GLFWwindow* window, double xpos, double ypos)
{
    auto spectrApp = getApp(window);
    spectrApp->cursorPositionCallback(window, xpos, ypos);
}

int SpectrDesktopApp::mainImpl(int argc, char* argv[])
{
    // parse input parameters
    // parseCommandLineArguments();

    // init logging
    spdlog::set_level(spdlog::level::trace);

    // setup graphics
    initGraphics();

    m_camera = std::make_shared<render_gl::Camera>();
    m_checkerGridRenderer = std::make_unique<render_gl::CheckerGridRenderer>();
    m_scalableTimeLineRenderer =
      std::make_unique<render_gl::ScalableAxisRenderer>(render_gl::AxisRenderMode::Horizontal);
    m_scalableFrequencyLineRenderer =
      std::make_unique<render_gl::ScalableAxisRenderer>(render_gl::AxisRenderMode::Vertical);
    m_renderContext = std::make_unique<render_gl::RenderContext>();
    m_renderContext->camera = m_camera;
    m_panTool = std::make_unique<render_gl::PanTool>(m_renderContext);
    m_fpsGuard = std::make_unique<render_gl::FpsGuard>();

    // create the spectrogram computation entities
    constexpr bool UseMockDataWorker = false;
    if (UseMockDataWorker)
    {
        // spectrogram settings
        const auto frequencyRangeSize = 100;
        const auto singleBufferColumnCount = 10;

        // create spectrogram container
        render_gl::TimeFrequencyHeatmapContainerSettings heatmapContainerSettings{
            .frequencyOffset = 0.0f,
            .valuesInOneHertz = 1.0f,
            .columnsInOneSecond = 1.0f,
            .columnHeightElementCount = frequencyRangeSize,
            .singleBufferColumnCount = singleBufferColumnCount,
        };
        m_timeFrequencyHeatmapContainer =
          std::make_shared<render_gl::TimeFrequencyHeatmapContainer>(heatmapContainerSettings);

        // create spectrogram worker (creator)
        using namespace std::chrono_literals;
        MockTimeFrequencyWorkerSettings timeFrequencyWorkerSettings{
            .updatePeriodSeconds = 1s,
            .updateWidthUnits = 1,
            .columnHeight = frequencyRangeSize,
            .heatmapContainer = m_timeFrequencyHeatmapContainer,
        };
        auto spectrogramWorker =
          std::make_shared<MockTimeFrequencyWorker>(timeFrequencyWorkerSettings);
        m_onMainLoopActions.push_back([spectrogramWorker = spectrogramWorker]()
                                      { spectrogramWorker->update(); });
    }
    else
    {
        auto openclManager = std::make_shared<calc_opencl::OpenclManager>();

#if defined(OS_WINDOWS)
        const std::vector<cl_context_properties> openclContextProperties{
            CL_GL_CONTEXT_KHR,
            reinterpret_cast<cl_context_properties>(glfwGetWGLContext(m_window)),
            CL_WGL_HDC_KHR,
            reinterpret_cast<cl_context_properties>(GetDC(glfwGetWin32Window(m_window))),
        };
#elif defined(OS_LINUX)
        const std::vector<cl_context_properties> openclContextProperties{
            CL_GL_CONTEXT_KHR,
            reinterpret_cast<cl_context_properties>(glfwGetGLXContext(m_window)),
            CL_GLX_DISPLAY_KHR,
            reinterpret_cast<cl_context_properties>(glfwGetX11Display()),
        };
#endif

        openclManager->initContext(openclContextProperties);

        // settings: FFT calculations per second
        const auto fftCalculationsInSecond = 100;
        // settings: FFT size
        const auto oneFftSampleCountPower = 17;
        const auto singleFftSize = 1 << oneFftSampleCountPower;

        auto fftCalculator = std::make_unique<calc_opencl::FftCooleyTukeyRadix2>(
          openclManager->getContext(), singleFftSize);

        // const auto assetPathRelative = "samples/1kHz_44100Hz_16bit_05sec.wav";
        // const auto assetPathRelative = "samples/Sample1.wav";
        const auto assetPathRelative = "samples/3.wav";
        // const auto assetPathRelative = "samples/440Hz_44100Hz_16bit_05sec.wav";
        const auto audioData = audio_loader::AudioLoader::loadAsset(assetPathRelative);
        // const auto audioData =
        //  audio_loader::AudioDataGenerator::generate<int16_t>(1 << 14, 30, { { 4, 32000 } });
        // audio_loader::AudioDataGenerator::generate<int16_t>(4096, 30, { { 23.1234, 32000 } });
        // audio_loader::AudioDataGenerator::generate<int16_t>(1024, 2, { { 4, 100 } });

        //---

        spdlog::info("Audio info:\n"
                     "sample rate: {}\n"
                     "duration: {}\n",
                     audioData.getSampleRate(),
                     audioData.getDuration());

        ASSERT(audioData.getSampleDataType() == audio_loader::SampleDataType::Int16);

        const auto columnsInOneSecond = fftCalculationsInSecond;

        const auto fftFrequencyRatio =
          static_cast<float>(audioData.getSampleRate()) / singleFftSize;

        const auto valuesPerHertzUnit = 1.0f / fftFrequencyRatio;
        // const auto valuesPerHertzUnit = singleFftSize / audioData.getDuration();

        const auto frequencyOffset = -fftFrequencyRatio / 2.0f;

        // create spectrogram container
        render_gl::TimeFrequencyHeatmapContainerSettings heatmapContainerSettings{
            .frequencyOffset = frequencyOffset,
            .valuesInOneHertz = valuesPerHertzUnit,
            .columnsInOneSecond = columnsInOneSecond,
            .columnHeightElementCount = singleFftSize / 2,
            .singleBufferColumnCount = 10, // now hardcoded empirical value
        };
        m_timeFrequencyHeatmapContainer =
          std::make_shared<render_gl::TimeFrequencyHeatmapContainer>(heatmapContainerSettings);

        AudioFileTimeFrequencyWorkerSettings audioFileWorkerSettings{
            .audioData = audioData,
            .oneFftSampleCount = singleFftSize,
            .fftCalculationsInSecond = fftCalculationsInSecond,
            .heatmapContainer = m_timeFrequencyHeatmapContainer,
            .fftCalculator = std::move(fftCalculator)
        };
        auto audioFileWorker =
          std::make_shared<AudioFileTimeFrequencyWorker>(std::move(audioFileWorkerSettings));

        m_onMainLoopActions.push_back([audioFileWorker = audioFileWorker]()
                                      { audioFileWorker->update(); });

        audioFileWorker->startWork();
    }

    // create spectrogram renderer
    m_timeFrequencyHeatmapRenderer =
      std::make_unique<render_gl::TimeFrequencyHeatmapRenderer>(m_timeFrequencyHeatmapContainer);

    // m_cameraBoundsController = std::make_shared<CameraBoundsController>();
    // m_onMainLoopActions.push_back(
    //   [=]()
    //   {
    //       //
    //   });

    // create widget with sliders
    auto minMaxWidget = std::make_shared<MinMaxWidget>();
    m_onRenderActions.push_back(
      [=,
       container = m_timeFrequencyHeatmapContainer](const render_gl::RenderContext& renderContext)
      {
          minMaxWidget->setRange(0, container->getMaxValue());
          minMaxWidget->render(renderContext);
      });

    m_onMainLoopActions.push_back(
      [minMaxWidget,
       container = m_timeFrequencyHeatmapContainer,
       renderer = m_timeFrequencyHeatmapRenderer]()
      {
          const auto minValue = std::max(minMaxWidget->getMin(), 0.0f);
          const auto maxValue = std::min(minMaxWidget->getMax(), container->getMaxValue());
          renderer->setScaleMinValue(minValue);
          renderer->setScaleMaxValue(maxValue);
      });

    // render!

    glfwShowWindow(m_window);
    while (!glfwWindowShouldClose(m_window))
    {
        onFrameStart();
        onMainLoopIteration();
        onBeforeRender();
        onRender();
        onAfterRender();
        onFrameEnd();
    }
    glfwTerminate();
    return EXIT_SUCCESS;
}

void SpectrDesktopApp::initGraphics()
{
    if (!glfwInit())
    {
        throw utils::Exception("Failed to init GLFW.");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, false);

    m_window = glfwCreateWindow(DefaultWindowWidth, DefaultWindowHeight, WindowTitle, NULL, NULL);
    ASSERT(m_window);

    int monitorWidth;
    int monitorHeight;
    glfwGetMonitorWorkarea(
      glfwGetPrimaryMonitor(), nullptr, nullptr, &monitorWidth, &monitorHeight);
    glfwSetWindowPos(
      m_window, (monitorWidth - DefaultWindowWidth) / 2, (monitorHeight - DefaultWindowHeight) / 2);

    glfwSetWindowUserPointer(m_window, this);
    glfwSetKeyCallback(m_window, keyborardCallbackGlobal);
    glfwSetScrollCallback(m_window, scrollCallbackGlobal);
    glfwSetMouseButtonCallback(m_window, mouseButtonCallbackGlobal);
    glfwSetCursorPosCallback(m_window, cursorPositionCallbackGlobal);

    glfwMakeContextCurrent(m_window);

    int version = gladLoadGL();
    if (version == 0)
    {
        throw utils::Exception("Failed to initialize OpenGL context\n");
    }

    render_gl::ImguiUtils::initImgui(m_window);
}

void SpectrDesktopApp::keyboardCallback(GLFWwindow* window,
                                        int key,
                                        int scancode,
                                        int action,
                                        int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    if (m_panTool && m_isPanToolEnabled)
    {
        m_panTool->onKeyboardInput(key, scancode, action, mods);
    }
}

void SpectrDesktopApp::scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (m_panTool && m_isPanToolEnabled)
    {
        m_panTool->onScroll(window, xoffset, yoffset);
    }
}

void SpectrDesktopApp::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (m_panTool && m_isPanToolEnabled)
    {
        m_panTool->onMouseButton(window, button, action, mods);
    }
}

void SpectrDesktopApp::cursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (m_panTool && m_isPanToolEnabled)
    {
        m_panTool->onCursorPosition(window, xpos, ypos);
    }
}

void SpectrDesktopApp::onFrameStart()
{
    m_fpsGuard->onFrameStart();

    const auto secondsToPixelCoeff =
      m_renderContext->viewportSize.x / (1.0f / m_camera->getScaleX());
    const auto frequencyToPixelCoeff =
      m_renderContext->viewportSize.y / (1.0f / m_camera->getScaleY());

    int width, height;
    glfwGetFramebufferSize(m_window, &width, &height);
    m_renderContext->viewportSize = { width, height };

    if (isResetScale)
    {
        setScale(m_secondToPixelCoeff, m_frequencyToPixelCoeff);
        isResetScale = false;
    }
    else
    {
        setScale(secondsToPixelCoeff, frequencyToPixelCoeff);
    }
}

void SpectrDesktopApp::onMainLoopIteration()
{
    for (auto& action : m_onMainLoopActions)
    {
        action();
    }
}

void SpectrDesktopApp::onBeforeRender()
{
    const auto& renderContext = *m_renderContext;

    glViewport(0, 0, renderContext.viewportSize.x, renderContext.viewportSize.y);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    glClearColor(0.5f, 0.5f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void SpectrDesktopApp::onRender()
{
    const auto& renderContext = *m_renderContext;

    for (auto& action : m_onRenderActions)
    {
        action(renderContext);
    }

    ImGui::ShowDemoWindow();

    ImGui::Begin("Hello, world!");
    ImGui::Text("This is some useful text.");
    ImGui::End();

    m_fpsGuard->onRender();
    m_checkerGridRenderer->render(*m_renderContext);
    m_scalableTimeLineRenderer->render(*m_renderContext);
    m_scalableFrequencyLineRenderer->render(*m_renderContext);
    m_timeFrequencyHeatmapRenderer->render(*m_renderContext);

    m_panTool->onFrame();
}

void SpectrDesktopApp::onAfterRender()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(m_window);
    glfwPollEvents();
}

void SpectrDesktopApp::onFrameEnd()
{
    m_fpsGuard->onFrameEnd();
}

void SpectrDesktopApp::setScale(float secondToPixelCoeff, float frequencyToPixelCoeff)
{
    const auto timeScale = 1.0f / (m_renderContext->viewportSize.x / secondToPixelCoeff);
    m_camera->setScaleX(timeScale);

    const auto frequencyScale = 1.0f / (m_renderContext->viewportSize.y / frequencyToPixelCoeff);
    m_camera->setScaleY(frequencyScale);
}

int SpectrDesktopApp::main(int argc, char* argv[])
{
    try
    {
        SpectrDesktopApp app;
        return app.mainImpl(argc, argv);
    }
    catch (const std::exception& ex)
    {
        spdlog::critical("Uncaught exception: {}", ex.what());
        return EXIT_FAILURE;
    }
    catch (...)
    {
        spdlog::critical("Uncaught non-exception throw.");
        return EXIT_FAILURE;
    }
}
}
