#include <spectr/desktop_app/WaterfallWindow.h>

#include <spectr/desktop_app/CameraWaterfallMover.h>
#include <spectr/desktop_app/MinMaxWidget.h>
#include <spectr/utils/Asset.h>

using namespace std::placeholders;

namespace spectr::desktop_app
{
namespace
{
constexpr auto SecondToPixelCoeff = 100.0f;
constexpr auto HertzToPixelCoeff = 1.0f / 1000.0f;
}

WaterfallWindow::WaterfallWindow(
  std::shared_ptr<Input> input,
  std::shared_ptr<render_gl::TimeFrequencyHeatmapContainer> container)
  : m_input{ input }
  , m_timeFrequencyHeatmapContainer{ container }
{
    // create fonts
    ImGuiIO& io = ImGui::GetIO();

    const auto axisFontPath = utils::Asset::getPath(UIFontAssetPath).string();
    m_axisFont = io.Fonts->AddFontFromFileTTF(axisFontPath.c_str(), 20.0f);

    const auto uiFontPath = axisFontPath;
    const auto uiFont = io.Fonts->AddFontFromFileTTF(axisFontPath.c_str(), 25.0f);

    // setup view
    m_camera = std::make_shared<render_gl::Camera>();

    // uncomment to enable checker grid rendering:
    // m_checkerGridRenderer = std::make_unique<render_gl::CheckerGridRenderer>();
    // m_onRenderActions.push_back(std::bind(&render_gl::CheckerGridRenderer::render,
    // m_checkerGridRenderer, _1));

    m_renderContext = std::make_unique<render_gl::RenderContext>();
    m_renderContext->camera = m_camera;
    m_panTool = std::make_unique<PanTool>(m_input, m_renderContext);

    // create spectrogram renderer
    m_heatmapRenderer =
      std::make_shared<render_gl::TimeFrequencyHeatmapRenderer>(m_timeFrequencyHeatmapContainer);

    m_onRenderActions.push_back(
      std::bind(&render_gl::TimeFrequencyHeatmapRenderer::render, m_heatmapRenderer, _1));

    // create camera automatic movement controller
    auto cameraWaterfallMover =
      std::make_shared<CameraWaterfallMover>(m_renderContext, m_timeFrequencyHeatmapContainer);
    m_onMainLoopActions.push_back(std::bind(&CameraWaterfallMover::update, cameraWaterfallMover));

    // create camera bounds controller
    m_cameraBoundsController = std::make_shared<CameraBoundsController>(m_renderContext);
    m_onMainLoopActions.push_back(
      [=, this]()
      {
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
      });

    // UI widget with heatmap visualizing settings
    HeatmapViewSettingsCallbacks heatmapViewSettingsCallbacks{
        .onMagnitudeColorModeChanged = [this](render_gl::MagnitudeColorMode newMagnitudeColorMode)
        { m_heatmapRenderer->setMagnitudeColorMode(newMagnitudeColorMode); },

        .onFrequencyAxisScaleChanged =
          [this](render_gl::FrequencyAxisScale newFrequencyAxisScale)
        {
            m_heatmapRenderer->setFrequencyAxisScale(newFrequencyAxisScale);
            recreateAxis();
        },

        .onMagnitudeAxisChanged = [this](render_gl::MagnitudeAxisScale newMagnitudeAxisScale)
        { m_heatmapRenderer->setMagnitudeAxisScale(newMagnitudeAxisScale); },

        .onMagnitudeInterpolationModeChanged =
          [this](render_gl::MagnitudeInterpolationMode newMagnitudeInterpolationMode)
        { m_heatmapRenderer->setMagnitudeInterpolationMode(newMagnitudeInterpolationMode); },

        .onWaterfallDirectionChanged =
          [=, this](render_gl::WaterfallDirection newWaterfallDirection)
        {
            isResetScale = true;
            cameraWaterfallMover->setWaterfallDirection(newWaterfallDirection);
            m_heatmapRenderer->setWaterfallDirectionMode(newWaterfallDirection);
            recreateAxis();
        },

        .onShowCursorInfoChanged = [this](bool isShowCursorInfo)
        { m_cursorInfoWidget->setEnabled(isShowCursorInfo); }
    };

    m_heatmapViewSettingsWidget =
      std::make_shared<HeatmapViewSettingsWidget>(uiFont, std::move(heatmapViewSettingsCallbacks));
    m_onRenderActions.push_back(
      std::bind(&HeatmapViewSettingsWidget::render, m_heatmapViewSettingsWidget, _1));

    m_heatmapViewSettingsWidget->setMagnitudeColorMode(
      render_gl::MagnitudeColorMode::FiveColorScale);
    m_heatmapViewSettingsWidget->setFrequencyAxisScale(render_gl::FrequencyAxisScale::Logarithmic);
    m_heatmapViewSettingsWidget->setMagnitudeAxisScale(render_gl::MagnitudeAxisScale::Logarithmic);
    m_heatmapViewSettingsWidget->setMagnitudeInterpolationMode(
      render_gl::MagnitudeInterpolationMode::Sharp);
    m_heatmapViewSettingsWidget->setWaterfallDirection(render_gl::WaterfallDirection::Horizontal);

    // create axis
    recreateAxis();

    m_onRenderActions.push_back(
      [this](const render_gl::RenderContext& renderContext)
      {
          for (auto& axis : m_axes)
          {
              axis->render(renderContext);
          }
      });

    // create scalogram min-max widget with sliders
    auto minMaxWidget = std::make_shared<MinMaxWidget>(uiFont);
    m_onRenderActions.push_back(
      [=, this](const render_gl::RenderContext& renderContext)
      {
          minMaxWidget->setRange(0, m_timeFrequencyHeatmapContainer->getMaxValue());
          minMaxWidget->render(renderContext);
      });

    m_onMainLoopActions.push_back(
      [minMaxWidget, this]()
      {
          const auto minValue = std::max(minMaxWidget->getMin(), 0.0f);
          const auto maxValue =
            std::min(minMaxWidget->getMax(), m_timeFrequencyHeatmapContainer->getMaxValue());
          m_heatmapRenderer->setScaleMinValue(minValue);
          m_heatmapRenderer->setScaleMaxValue(maxValue);
      });

    // widget showing info about the point where mouse points
    auto worldPointToTextCallback = [this](const glm::vec2& worldPoint)
    {
        float time = worldPoint.x;
        float frequencyCoord = worldPoint.y;
        if (m_heatmapViewSettingsWidget->getWaterfallDirection() ==
            render_gl::WaterfallDirection::Vertical)
        {
            std::swap(time, frequencyCoord);
            time *= -1;
        }

        std::stringstream ss;
        ss << "Time: " << time << " sec.\n";
        ss << "Frequency: ";

        const auto& settings = m_timeFrequencyHeatmapContainer->getSettings();
        if (m_heatmapViewSettingsWidget->getFrequencyAxisScale() ==
            render_gl::FrequencyAxisScale::Linear)
        {
            ss << frequencyCoord << " Hz\n";
        }
        else
        {
            const auto maxFrequency = m_timeFrequencyHeatmapContainer->getFrequencyRange().max;
            const auto frequency =
              render_gl::LogScaleUtils::getFrequency(frequencyCoord, maxFrequency);
            ss << frequency << " Hz\n";
        }

        return ss.str();
    };

    m_cursorInfoWidget =
      std::make_shared<HeatmapCursorInfo>(m_input, uiFont, std::move(worldPointToTextCallback));
    m_onRenderActions.push_back(std::bind(&HeatmapCursorInfo::onRender, m_cursorInfoWidget, _1));
    m_onMainLoopActions.push_back(
      [=, this]()
      {
          if (m_input->isKeyDown(GLFW_KEY_Q))
          {
              m_heatmapViewSettingsWidget->setShowCursorInfo(
                !m_heatmapViewSettingsWidget->getShowCursorInfo());
          }
      });
}

void WaterfallWindow::recreateAxis()
{
    m_axes.clear();

    glm::vec2 timeAxisDirection;
    glm::vec2 frequencyAxisDirection;

    render_gl::AxisDockPosition timeAxisDock1;
    render_gl::AxisDockPosition timeAxisDock2;

    render_gl::AxisDockPosition frequencyAxisDock1;
    render_gl::AxisDockPosition frequencyAxisDock2;

    const auto waterfallDirection = m_heatmapViewSettingsWidget->getWaterfallDirection();
    if (waterfallDirection == render_gl::WaterfallDirection::Horizontal)
    {
        timeAxisDock1 = render_gl::AxisDockPosition::Bottom;
        timeAxisDock2 = render_gl::AxisDockPosition::Top;

        frequencyAxisDock1 = render_gl::AxisDockPosition::Left;
        frequencyAxisDock2 = render_gl::AxisDockPosition::Right;

        timeAxisDirection = glm::vec2(1, 0);
        frequencyAxisDirection = glm::vec2(0, 1);
    }
    else if (waterfallDirection == render_gl::WaterfallDirection::Vertical)
    {
        timeAxisDock1 = render_gl::AxisDockPosition::Left;
        timeAxisDock2 = render_gl::AxisDockPosition::Right;

        frequencyAxisDock1 = render_gl::AxisDockPosition::Bottom;
        frequencyAxisDock2 = render_gl::AxisDockPosition::Top;

        timeAxisDirection = glm::vec2(0, -1);
        frequencyAxisDirection = glm::vec2(1, 0);
    }

    // create frequency axis
    std::shared_ptr<render_gl::AxisRenderer> frequencyAxisBig1;
    std::shared_ptr<render_gl::AxisRenderer> frequencyAxisSmall1;
    std::shared_ptr<render_gl::AxisRenderer> frequencyAxisBig2;
    std::shared_ptr<render_gl::AxisRenderer> frequencyAxisSmall2;

    const auto frequencyAxisScale = m_heatmapViewSettingsWidget->getFrequencyAxisScale();
    if (frequencyAxisScale == render_gl::FrequencyAxisScale::Linear)
    {
        frequencyAxisBig1 = std::make_shared<render_gl::AxisRenderer>(
          frequencyAxisDirection, frequencyAxisDock1, m_axisFont);

        frequencyAxisSmall1 = std::make_shared<render_gl::AxisRenderer>(
          frequencyAxisDirection, frequencyAxisDock1, m_axisFont, 1);

        frequencyAxisBig2 = std::make_shared<render_gl::AxisRenderer>(
          frequencyAxisDirection, frequencyAxisDock2, m_axisFont);

        frequencyAxisSmall2 = std::make_shared<render_gl::AxisRenderer>(
          frequencyAxisDirection, frequencyAxisDock2, m_axisFont, 1);
    }
    else if (frequencyAxisScale == render_gl::FrequencyAxisScale::Logarithmic)
    {
        const auto frequencyRange = m_timeFrequencyHeatmapContainer->getFrequencyRange();

        frequencyAxisBig1 = std::make_shared<render_gl::LogarithmicAxisRenderer>(
          frequencyRange, frequencyAxisDirection, frequencyAxisDock1, m_axisFont);

        frequencyAxisSmall1 = std::make_shared<render_gl::LogarithmicAxisRenderer>(
          frequencyRange, frequencyAxisDirection, frequencyAxisDock1, m_axisFont, 1);

        frequencyAxisBig2 = std::make_shared<render_gl::LogarithmicAxisRenderer>(
          frequencyRange, frequencyAxisDirection, frequencyAxisDock2, m_axisFont);

        frequencyAxisSmall2 = std::make_shared<render_gl::LogarithmicAxisRenderer>(
          frequencyRange, frequencyAxisDirection, frequencyAxisDock2, m_axisFont, 1);
    }

    frequencyAxisBig1->setLabelSuffix(" Hz");
    frequencyAxisBig2->setLabelSuffix(" Hz");

    frequencyAxisSmall1->setIsRenderLabels(false);
    frequencyAxisSmall1->setMarkHeight(frequencyAxisBig1->getMarkHeight() / 2.0f);

    frequencyAxisSmall2->setIsRenderLabels(false);
    frequencyAxisSmall2->setMarkHeight(frequencyAxisBig2->getMarkHeight() / 2.0f);

    m_axes.push_back(frequencyAxisBig1);
    m_axes.push_back(frequencyAxisSmall1);

    m_axes.push_back(frequencyAxisBig2);
    m_axes.push_back(frequencyAxisSmall2);

    // create time axis
    auto timeAxisBig1 =
      std::make_shared<render_gl::AxisRenderer>(timeAxisDirection, timeAxisDock1, m_axisFont);

    auto timeAxisSmall1 =
      std::make_shared<render_gl::AxisRenderer>(timeAxisDirection, timeAxisDock1, m_axisFont, 1);

    auto timeAxisBig2 =
      std::make_shared<render_gl::AxisRenderer>(timeAxisDirection, timeAxisDock2, m_axisFont);

    auto timeAxisSmall2 =
      std::make_shared<render_gl::AxisRenderer>(timeAxisDirection, timeAxisDock2, m_axisFont, 1);

    timeAxisSmall1->setIsRenderLabels(false);
    timeAxisSmall1->setMarkHeight(timeAxisBig1->getMarkHeight() / 2.0f);

    timeAxisSmall2->setIsRenderLabels(false);
    timeAxisSmall2->setMarkHeight(timeAxisBig2->getMarkHeight() / 2.0f);

    m_axes.push_back(timeAxisBig1);
    m_axes.push_back(timeAxisSmall1);

    m_axes.push_back(timeAxisBig2);
    m_axes.push_back(timeAxisSmall2);
}

void WaterfallWindow::setScale(float secondToPixelCoeff, float frequencyToPixelCoeff)
{
    const auto timeScale = 1.0f / (m_renderContext->viewportSize.x / secondToPixelCoeff);
    m_camera->setScaleX(timeScale);

    const auto frequencyScale = 1.0f / (m_renderContext->viewportSize.y / frequencyToPixelCoeff);
    m_camera->setScaleY(frequencyScale);
}

void WaterfallWindow::onMainLoopUpdate()
{
    const auto secondsToPixelCoeff =
      m_renderContext->viewportSize.x / (1.0f / m_camera->getScaleX());
    const auto frequencyToPixelCoeff =
      m_renderContext->viewportSize.y / (1.0f / m_camera->getScaleY());

    m_renderContext->viewportSize = { getWidth(), getHeight() };

    if (isResetScale)
    {
        const auto waterfallDirection = m_heatmapViewSettingsWidget->getWaterfallDirection();
        if (waterfallDirection == render_gl::WaterfallDirection::Horizontal)
        {
            setScale(SecondToPixelCoeff, HertzToPixelCoeff);
        }
        else
        {
            setScale(HertzToPixelCoeff, SecondToPixelCoeff);
        }

        isResetScale = false;
    }
    else
    {
        setScale(secondsToPixelCoeff, frequencyToPixelCoeff);
    }

    for (auto& action : m_onMainLoopActions)
    {
        action();
    }
}

void WaterfallWindow::onBeforeRender()
{
    Window::onBeforeRender();
}

void WaterfallWindow::onRender()
{
    Window::onRender();

    for (auto& action : m_onRenderActions)
    {
        action(*m_renderContext);
    }
}

void WaterfallWindow::onAfterRender()
{
    Window::onAfterRender();

    m_panTool->update();
}
}
