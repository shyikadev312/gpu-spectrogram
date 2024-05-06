#include <spectr/desktop_app/SpectrDesktopApp.h>

#include <spectr/audio_loader/AudioLoader.h>
#include <spectr/audio_loader/SignalDataGenerator.h>
#include <spectr/calc_opencl/FftCooleyTukeyRadix2CL.h>
#include <spectr/calc_opencl/OpenclApi.h>
#include <spectr/calc_opencl/OpenclManager.h>
#include <spectr/calc_opencl/RtsaUpdater.h>
#include <spectr/desktop_app/AudioFileTimeFrequencyWorker.h>
#include <spectr/desktop_app/CmdArgumentParser.h>
#include <spectr/desktop_app/MockTimeFrequencyWorker.h>
#include <spectr/render_gl/ImguiUtils.h>
#include <spectr/utils/Asset.h>
#include <spectr/utils/Assert.h>
#include <spectr/utils/Exception.h>
#include <spectr/utils/Version.h>
#include <spectr/real_time_input/FileInput.h>
#include <spectr/real_time_input/RealTimeInputPortAudio.h>
#include <spectr/real_time_input/RealTimeInputBladeRF.h>

// #include <spdlog/spdlog.h>

#include <iostream>
#include <limits>

namespace spectr::desktop_app
{
namespace
{
constexpr auto DefaultWindowWidth = 1920;
constexpr auto DefaultWindowHeight = 1080;
const auto WindowTitle = "Spectrogram renderer";

std::vector<cl_context_properties> getOpenCLContextProperties(GLFWwindow* window)
{
#ifdef _WIN32
    const std::vector<cl_context_properties> openclContextProperties{
        CL_GL_CONTEXT_KHR,
        reinterpret_cast<cl_context_properties>(glfwGetWGLContext(window)),
        CL_WGL_HDC_KHR,
        reinterpret_cast<cl_context_properties>(GetDC(glfwGetWin32Window(window))),
    };
#elif defined(OS_LINUX)
    const std::vector<cl_context_properties> openclContextProperties{
        CL_GL_CONTEXT_KHR,
        reinterpret_cast<cl_context_properties>(glfwGetGLXContext(window)),
        CL_GLX_DISPLAY_KHR,
        reinterpret_cast<cl_context_properties>(glfwGetX11Display()),
    };
#endif
    return openclContextProperties;
}

DesktopAppSettings parseCommandLineArguments(int argc, const char* argv[])
{
    constexpr bool HardcodeInput = true;
    if (!HardcodeInput)
    {
        return CmdArgumentParser::parse(argc, argv);
    }

    // spdlog::warn("Using hardcoded input values!");
    std::cout << "Using hardcoded input values!";

    DesktopAppSettings settings;
    settings.command = Command::Execute;
    settings.fftCalculationPerSecond = 32;

    const auto powerOfTwo = 16u;
    settings.fftSize = 1ull << powerOfTwo;

    // const auto assetPathRelative = "samples/1.wav";
    // const auto assetPathRelative = "samples/2.wav";
    // const auto assetPathRelative = "samples/3.wav";

    // const auto assetPathRelative = "samples/100Hz_44100Hz_16bit_05sec.wav";
    // const auto assetPathRelative = "samples/250Hz_44100Hz_16bit_05sec.wav";
    // const auto assetPathRelative = "samples/440Hz_44100Hz_16bit_05sec.wav";
    // const auto assetPathRelative = "samples/1kHz_44100Hz_16bit_05sec.wav";
    // const auto assetPathRelative = "samples/10kHz_44100Hz_16bit_05sec.wav";

    const auto assetPathRelative = "samples/always-with-me-always-with-you-piano-long-21257.wav";
    // const auto assetPathRelative = "samples/anima-main-7426.wav";
    // const auto assetPathRelative =
    // "samples/epic-background-orchestral-music-type-hip-hop-beat-legacy-of-vivaldi-143986.wav";
    // const auto assetPathRelative = "samples/smooth-ac-guitar-loop-93bpm-137706.wav";

    // const auto assetPathRelative = "samples/Silence.wav";

    const auto audioFilePath = utils::Asset::getPath(assetPathRelative);
    settings.audioFilePath = audioFilePath;

    // settings.source = AudioSource::File;
    settings.source = AudioSource::BladeRF;

    return settings;
}
}

int SpectrDesktopApp::main(int argc, const char* argv[])
{
    try
    {
        SpectrDesktopApp app;
        return app.mainImpl(argc, argv);
    }
    catch (const cl::Error& err)
    {
        std::cerr << "Uncaught OpenCL exception: " << err.what() << "(" << err.err() << ")" << std::endl;
        return EXIT_FAILURE;
    }
    catch (const std::exception& ex)
    {
        // spdlog::critical("Uncaught exception: {}", ex.what());
        std::cerr << "Uncaught exception: " << ex.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (...)
    {
        // spdlog::critical("Uncaught non-exception throw.");
        std::cerr << "Uncaught non-exception throw." << std::endl;
        return EXIT_FAILURE;
    }
}

int SpectrDesktopApp::mainImpl(int argc, const char* argv[])
{
    // spdlog::set_level(spdlog::level::debug);
    const auto settings = parseCommandLineArguments(argc, argv);

    if (settings.command == Command::PrintHelp)
    {
        std::cout << settings.helpDescription;
        return EXIT_SUCCESS;
    }

    if (settings.command == Command::PrintVersion)
    {
        utils::Version::print(std::cout, "Spectr signal analysis");
        return EXIT_SUCCESS;
    }

    // spdlog::info("Spectr launched.");
    // spdlog::info("Input signal file: {}", settings.audioFilePath);
    // spdlog::info("FFT size: {}", settings.fftSize);
    // spdlog::info("FFT calculations per second: {}", settings.fftCalculationPerSecond);
    std::cout << "Spectr launched.";
    std::cout << "Input signal file: " << settings.audioFilePath;
    std::cout << "FFT size: " << settings.fftSize;
    std::cout << "FFT calculations per second: " << settings.fftCalculationPerSecond;

    switch (settings.source) {
        case AudioSource::File:
            m_inputSource = std::make_shared<real_time_input::FileInput>(settings.audioFilePath);
            break;
        case AudioSource::PortAudio:
            m_inputSource = std::make_shared<real_time_input::RealTimeInputPortAudio>();
            break;
        case AudioSource::BladeRF:
            m_inputSource = std::make_shared<real_time_input::RealTimeInputBladeRF>();
            break;
    }

    initGraphics();
    initFftCalculator(settings);
    m_waterfallWindow = std::make_unique<WaterfallWindow>(m_input, m_timeFrequencyHeatmapContainer);
    m_rtsaWindow = std::make_unique<RtsaWindow>(m_input, m_rtsaHeatmapContainer);
    m_splitWindow = std::make_unique<SplitWindow>(m_input, m_timeFrequencyHeatmapContainer, m_rtsaHeatmapContainer);
    m_currentWindow = m_waterfallWindow;

    // create fonts
    ImGuiIO& io = ImGui::GetIO();
    const auto uiFontPath = utils::Asset::getPath(UIFontAssetPath).string();
    auto uiFont = io.Fonts->AddFontFromFileTTF(uiFontPath.c_str(), 20.0f);
    m_fpsGuard = std::make_unique<render_gl::FpsGuard>(uiFont);

    glfwShowWindow(m_window);
    while (!glfwWindowShouldClose(m_window))
    {
        m_input->newFrame();
        glfwPollEvents();

        m_fpsGuard->onFrameStart();

        int width, height;
        glfwGetFramebufferSize(m_window, &width, &height);
        glViewport(0, 0, width, height);
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        /* ImGui::ShowDemoWindow();
         ImGui::Begin("Hello, world!");
         ImGui::Text("This is some useful text.");
         ImGui::End();*/

        for (auto& action : m_onMainLoopActions)
        {
            action();
        }
        m_currentWindow->setSize(width, height);
        m_currentWindow->onMainLoopUpdate();
        m_currentWindow->onBeforeRender();
        m_currentWindow->onRender();

        m_fpsGuard->onRender();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        m_currentWindow->onAfterRender();

        if (m_input->isKeyPressed(GLFW_KEY_1))
        {
            m_currentWindow = m_waterfallWindow;
        }
        if (m_input->isKeyPressed(GLFW_KEY_2))
        {
            m_currentWindow = m_rtsaWindow;
        }
        if (m_input->isKeyPressed(GLFW_KEY_3))
        {
            m_currentWindow = m_splitWindow;
        }
        if (m_input->isKeyPressed(GLFW_KEY_ESCAPE))
        {
            glfwSetWindowShouldClose(m_window, GLFW_TRUE);
        }

        glfwSwapBuffers(m_window);
        m_fpsGuard->onFrameEnd();
    }

    return EXIT_SUCCESS;
}

void SpectrDesktopApp::initGraphics()
{
    m_glfwApi = std::make_unique<render_gl::GlfwWrapper>();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, false);

    m_window = std::move(render_gl::GlfwWindowWrapper(
      DefaultWindowWidth, DefaultWindowHeight, WindowTitle, NULL, NULL));

    m_input = std::make_shared<Input>(m_window);

    // set window position to the screen center
    int monitorWidth;
    int monitorHeight;
    glfwGetMonitorWorkarea(
      glfwGetPrimaryMonitor(), nullptr, nullptr, &monitorWidth, &monitorHeight);
    glfwSetWindowPos(
      m_window, (monitorWidth - DefaultWindowWidth) / 2, (monitorHeight - DefaultWindowHeight) / 2);

    glfwMakeContextCurrent(m_window.getWindow());
    glfwSwapInterval(0);

    int version = gladLoadGL();
    if (version == 0)
    {
        throw utils::Exception("Failed to initialize OpenGL context\n");
    }

    render_gl::ImguiUtils::initImgui(m_window);
}

void SpectrDesktopApp::initFftCalculator(const DesktopAppSettings& settings)
{
    auto frequenciesCount = settings.fftSize / 2;

    const auto waterfallHistoryTime = 300.0f; // 5 minutes
    const auto singleBufferColumnCount = 10;
    const auto maxBufferCount = static_cast<size_t>(
      (waterfallHistoryTime * settings.fftCalculationPerSecond) / singleBufferColumnCount);

    constexpr bool UseMockDataWorker = false;
    if (UseMockDataWorker)
    {
        // spectrogram settings
        const auto frequencyRangeSize = 100;

        // create spectrogram container
        render_gl::TimeFrequencyHeatmapContainerSettings heatmapContainerSettings{
            .frequencyOffset = 0.0f,
            .valuesInOneHertz = 1.0f,
            .columnsInOneSecond = 1.0f,
            .columnHeightElementCount = frequencyRangeSize,
            .singleBufferColumnCount = singleBufferColumnCount,
            .maxBuffersCount = 1000,
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
            .rtsaHeatmapContainer = m_rtsaHeatmapContainer,
        };
        auto spectrogramWorker =
          std::make_shared<MockTimeFrequencyWorker>(timeFrequencyWorkerSettings);
        m_onMainLoopActions.push_back([spectrogramWorker = spectrogramWorker]()
                                      { spectrogramWorker->update(); });
    }
    else
    {
        const auto openglContextProperties = getOpenCLContextProperties(m_window);
        auto openclManager = std::make_shared<calc_opencl::OpenclManager>(openglContextProperties);

        // const auto audioData =
        //   //
        //   audio_loader::AudioLoader::load(settings.audioFilePath);
        /*audio_loader::AudioDataGenerator::generateWithConstant<int16_t>(
          1 << 14, 30, std::numeric_limits<int16_t>::max());*/
        // audio_loader::AudioDataGenerator::generate<int16_t>(1 << 14, 30, { { 4, 32000 } });
        //  audio_loader::AudioDataGenerator::generate<int16_t>(4096, 30, { { 23.1234, 32000 } });
        //  audio_loader::AudioDataGenerator::generate<int16_t>(1024, 2, { { 4, 100 } });

        // spdlog::info("Loaded signal data:\n"
        //              "\tSample rate: {} Hz\n"
        //              "\tDuration: {} seconds\n",
        //              audioData.getSampleRate(),
        //              audioData.getDuration());

        // std::cout << "Loaded signal data:\n"
        //              "\tSample rate: " << audioData.getSampleRate() << " Hz\n"
        //              "\tDuration: " << audioData.getDuration() << " seconds" << std::endl;
        // 
        // ASSERT(audioData.getSampleDataType() == audio_loader::SampleDataType::Int16);

        const auto columnsInOneSecond = settings.fftCalculationPerSecond;

        const auto fftFrequencyRatio =
          static_cast<float>(m_inputSource->getSampleRate()) / settings.fftSize;

        const auto valuesPerHertzUnit = 1.0f / fftFrequencyRatio;

        const auto frequencyOffset = -fftFrequencyRatio / 2.0f;

        auto fftCalculator = std::make_unique<calc_opencl::FftCooleyTukeyRadix2>(
          openclManager->getContext(), settings.fftSize);

        // create spectrogram container
        render_gl::TimeFrequencyHeatmapContainerSettings heatmapContainerSettings{
            .frequencyOffset = frequencyOffset,
            .valuesInOneHertz = valuesPerHertzUnit,
            .columnsInOneSecond = static_cast<float>(columnsInOneSecond),
            .columnHeightElementCount = frequenciesCount,
            .singleBufferColumnCount = singleBufferColumnCount,
            .maxBuffersCount = maxBufferCount,
        };
        m_timeFrequencyHeatmapContainer =
          std::make_shared<render_gl::TimeFrequencyHeatmapContainer>(heatmapContainerSettings);

        const auto magnitudeDbfsRange = 96.0f;       // for 16-bit signal
        const auto magnitudeRangeValuesCount = 1024; // empiric arbitrary value;
         
        // RTSA
        const render_gl::RtsaContainerSettings rtsaContainerSettings{
            .frequencyValuesCount = frequenciesCount,
            .magnitudeDecibelRange = magnitudeDbfsRange,
            .magnitudeRangeValuesCount = magnitudeRangeValuesCount,
            .valuesInOneHertz = valuesPerHertzUnit,
        };
        m_rtsaHeatmapContainer = std::make_shared<render_gl::RtsaContainer>(rtsaContainerSettings);

        const auto rtsaHistoryDuration = 2.0f; // hardcoded empiric value, can be changed
        const auto rtsaHistoryBufferCount =
          static_cast<size_t>(settings.fftCalculationPerSecond * rtsaHistoryDuration);

        auto rtsaUpdater = std::make_unique<calc_opencl::RtsaUpdater>(
          openclManager->getContext(),
          rtsaContainerSettings.frequencyValuesCount,
          rtsaContainerSettings.magnitudeRangeValuesCount,
          rtsaHistoryBufferCount,
          magnitudeDbfsRange,
          rtsaContainerSettings.frequencyValuesCount * rtsaContainerSettings.magnitudeRangeValuesCount * sizeof(float) * 2);

        // worker
        AudioFileTimeFrequencyWorkerSettings audioFileWorkerSettings{
            .source = m_inputSource,
            .oneFftSampleCount = settings.fftSize,
            .fftCalculationsInSecond = settings.fftCalculationPerSecond,
            .heatmapContainer = m_timeFrequencyHeatmapContainer,
            .rtsaHeatmapContainer = m_rtsaHeatmapContainer,
            .fftCalculator = std::move(fftCalculator),
            .rtsaUpdater = std::move(rtsaUpdater),
            .rtsaBufferSize = rtsaContainerSettings.frequencyValuesCount * rtsaContainerSettings.magnitudeRangeValuesCount * sizeof(float) * 2,
            .fftSize = settings.fftSize
        };

        auto audioFileWorker =
          std::make_shared<AudioFileTimeFrequencyWorker>(std::move(audioFileWorkerSettings));

        m_onMainLoopActions.push_back(
          std::bind(&AudioFileTimeFrequencyWorker::update, audioFileWorker));

        audioFileWorker->startWork();
    }
}
}
