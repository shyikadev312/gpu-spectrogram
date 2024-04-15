#include <spectr/desktop_app/SplitWindow.h>

#include <spectr/desktop_app/CameraWaterfallMover.h>
#include <spectr/desktop_app/MinMaxWidget.h>
#include <spectr/utils/Asset.h>

using namespace std::placeholders;

namespace spectr::desktop_app {
    namespace {
        constexpr auto SecondToPixelCoeff = 100.0f;
        constexpr auto HertzToPixelCoeff = 1.0f / 1000.0f;
    }

    SplitWindow::SplitWindow(
        std::shared_ptr<Input> input,
        std::shared_ptr<render_gl::TimeFrequencyHeatmapContainer> container,
        std::shared_ptr<render_gl::RtsaContainer> rtsaContainer)
        : m_input { input }
        , m_timeFrequencyHeatmapContainer { container }
        , m_rtsaContainer { rtsaContainer } {
        // create fonts
        ImGuiIO& io = ImGui::GetIO();

        const auto axisFontPath = utils::Asset::getPath(UIFontAssetPath).string();
        m_axisFont = io.Fonts->AddFontFromFileTTF(axisFontPath.c_str(), 20.0f);

        const auto uiFontPath = axisFontPath;
        const auto uiFont = io.Fonts->AddFontFromFileTTF(axisFontPath.c_str(), 25.0f);

        // setup view
        m_camera = std::make_shared<render_gl::Camera>();
        m_rtsaCamera = std::make_shared<render_gl::Camera>();

        m_rtsaCamera->setScaleX(1.0f / 11000.0f);
        m_rtsaCamera->setScaleY(1.0f / 95);
        m_rtsaCamera->setPositionWorld({ 11000, -95 });

        // uncomment to enable checker grid rendering:
        // m_checkerGridRenderer = std::make_unique<render_gl::CheckerGridRenderer>();
        // m_onRenderActions.push_back(std::bind(&render_gl::CheckerGridRenderer::render,
        // m_checkerGridRenderer, _1));

        m_renderContext = std::make_unique<render_gl::RenderContext>();
        m_renderContext->camera = m_camera;
        m_rtsaRenderContext = std::make_unique<render_gl::RenderContext>();
        m_rtsaRenderContext->camera = m_rtsaCamera;
        m_panTool = std::make_unique<PanTool>(m_input, m_renderContext);

        // create spectrogram renderer
        m_heatmapRenderer =
            std::make_shared<render_gl::TimeFrequencyHeatmapRenderer>(m_timeFrequencyHeatmapContainer);

        m_onRenderActions.push_back(
            std::bind(&render_gl::TimeFrequencyHeatmapRenderer::render, m_heatmapRenderer, _1));

        m_rtsaRenderer = std::make_shared<render_gl::RtsaRenderer>(m_rtsaContainer);

        // create camera automatic movement controller
        auto cameraWaterfallMover =
            std::make_shared<CameraWaterfallMover>(m_renderContext, m_timeFrequencyHeatmapContainer);
        m_onMainLoopActions.push_back(std::bind(&CameraWaterfallMover::update, cameraWaterfallMover));

        // create camera bounds controller
        m_cameraBoundsController = std::make_shared<CameraBoundsController>(m_renderContext);
        m_onMainLoopActions.push_back(
            [=, this]() {
                const auto rangeX = m_timeFrequencyHeatmapContainer->getTimeRange();
                const auto rangeY = m_timeFrequencyHeatmapContainer->getFrequencyRange();

                const glm::vec3 lowerLeft{ rangeX.min, rangeY.min, 1.0f };
                const glm::vec3 upperRight{ rangeX.max, rangeY.max, 1.0f };

                const auto rotationMatrix = m_heatmapRenderer->getRotationMatrix();

                const auto p1 = rotationMatrix * lowerLeft;
                const auto p2 = rotationMatrix * upperRight;

                const auto minX = std::min(p1.x, p2.x);
                const auto maxX = std::max(p1.x, p2.x);
                const auto minY = std::min(p1.y, p2.y);
                const auto maxY = std::max(p1.y, p2.y);

                const glm::vec2 finalLowerLeft{ minX, minY };
                const glm::vec2 finalUpperRight{ maxX, maxY };

                m_cameraBoundsController->setBounds(finalLowerLeft, finalUpperRight);

                m_cameraBoundsController->update();
            }
        );

        cameraWaterfallMover->setWaterfallDirection(render_gl::WaterfallDirection::Vertical);
        m_heatmapRenderer->setWaterfallDirectionMode(render_gl::WaterfallDirection::Vertical);
        // m_heatmapRenderer->setFrequencyAxisScale(render_gl::FrequencyAxisScale::Logarithmic);
        m_heatmapRenderer->setMagnitudeAxisScale(render_gl::MagnitudeAxisScale::Logarithmic);

        // create axis
        recreateAxis();

        m_onRenderActions.push_back(
            [this](const render_gl::RenderContext& renderContext) {
                for (auto& axis : m_axes)
                {
                    axis->render(renderContext);
                }
            });

        // create scalogram min-max widget with sliders
        auto minMaxWidget = std::make_shared<MinMaxWidget>(uiFont);
        m_onRenderActions.push_back(
            [=, this](const render_gl::RenderContext& renderContext) {
                minMaxWidget->setRange(0, m_timeFrequencyHeatmapContainer->getMaxValue());
                minMaxWidget->render(renderContext);
            });

        m_onMainLoopActions.push_back(
            [minMaxWidget, this]() {
                const auto minValue = std::max(minMaxWidget->getMin(), 0.0f);
                const auto maxValue =
                    std::min(minMaxWidget->getMax(), m_timeFrequencyHeatmapContainer->getMaxValue());
                m_heatmapRenderer->setScaleMinValue(minValue);
                m_heatmapRenderer->setScaleMaxValue(maxValue);
            });

        // widget showing info about the point where mouse points
        auto worldPointToTextCallback = [this](const glm::vec2& worldPoint) {
            float time = worldPoint.y * -1 + std::max(10.0f, m_timeFrequencyHeatmapContainer->getTimeRange().max);
            float frequencyCoord = worldPoint.x;

            std::stringstream ss;

            if (time >= 0.0f) {
                // Heatmap widget
                ss << "Time: " << time << " sec.\n";
                ss << "Frequency: ";

                const auto& settings = m_timeFrequencyHeatmapContainer->getSettings();
                const auto maxFrequency = m_timeFrequencyHeatmapContainer->getFrequencyRange().max;
                const auto frequency =
                    render_gl::LogScaleUtils::getFrequency(frequencyCoord, maxFrequency);
                ss << frequency << " Hz\n";

            } else {
                // RTSA widget
                float frequencyCoord = worldPoint.x;

                // todo: fix this number
                float dbfs = worldPoint.y;

                ss << std::format("Frequency: {}", frequencyCoord) << std::endl;
                ss << std::format("dBFS: {}", dbfs) << std::endl;
            }

            return ss.str();
        };

        m_cursorInfoWidget =
            std::make_shared<HeatmapCursorInfo>(m_input, uiFont, std::move(worldPointToTextCallback));
        m_onRenderActions.push_back(std::bind(&HeatmapCursorInfo::onRender, m_cursorInfoWidget, _1));
    }

    void SplitWindow::recreateAxis() { }

    void SplitWindow::setScale(float secondToPixelCoeff, float frequencyToPixelCoeff) {
        const auto timeScale = 1.0f / (m_renderContext->viewportSize.x / secondToPixelCoeff);
        m_camera->setScaleX(timeScale);

        const auto frequencyScale = 1.0f / (m_renderContext->viewportSize.y / frequencyToPixelCoeff);
        m_camera->setScaleY(frequencyScale);
    }

    void SplitWindow::onMainLoopUpdate() {
        const auto secondsToPixelCoeff =
            m_renderContext->viewportSize.x / (1.0f / m_camera->getScaleX());
        const auto frequencyToPixelCoeff =
            m_renderContext->viewportSize.y / (1.0f / m_camera->getScaleY());

        m_renderContext->viewportSize = { getWidth(), getHeight() / 2 };
        m_rtsaRenderContext->viewportSize = { getWidth(), getHeight() / 2 };

        if (isResetScale)
        {
            setScale(HertzToPixelCoeff, SecondToPixelCoeff);

            isResetScale = false;
        } else
        {
            setScale(secondsToPixelCoeff, frequencyToPixelCoeff);
        }

        for (auto& action : m_onMainLoopActions)
        {
            action();
        }
    }

    void SplitWindow::onBeforeRender() {
        m_camera->setScaleY(0.1f);

        auto pos = m_camera->getPositionWorld();
        auto max = m_timeFrequencyHeatmapContainer->getTimeRange().max;

        pos.y = std::min(0.0f, 10.0f - max);
        m_camera->setPositionWorld(pos);

        Window::onBeforeRender();
    }

    void SplitWindow::onRender() {
        Window::onRender();

        for (auto& action : m_onRenderActions)
        {
            action(*m_renderContext);
        }

        m_rtsaRenderer->render(*m_rtsaRenderContext);
    }

    void SplitWindow::onAfterRender() {
        Window::onAfterRender();

        m_panTool->update();
    }
}
