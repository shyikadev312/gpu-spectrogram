#include <spectr/desktop_app/RtsaWindow.h>

#include <spectr/desktop_app/CameraBoundsController.h>
#include <spectr/desktop_app/RtsaViewSettingsWidget.h>
#include <spectr/utils/Asset.h>

#include <fmt/format.h>

using namespace std::placeholders;

namespace spectr::desktop_app
{
RtsaWindow::RtsaWindow(std::shared_ptr<Input> input,
                       std::shared_ptr<render_gl::RtsaContainer> container)
  : m_input{ input }
  , m_container{ container }
{
    // create fonts
    ImGuiIO& io = ImGui::GetIO();

    const auto axisFontPath = utils::Asset::getPath(UIFontAssetPath).string();
    m_axisFont = io.Fonts->AddFontFromFileTTF(axisFontPath.c_str(), 20.0f);

    const auto uiFontPath = axisFontPath;
    const auto uiFont = io.Fonts->AddFontFromFileTTF(axisFontPath.c_str(), 25.0f);

    // create render components
    m_camera = std::make_shared<render_gl::Camera>();

    m_camera->setScaleX(1.0f / 10000.0f);
    m_camera->setScaleY(1.0f / 50.0f);
    m_camera->setPositionWorld({ 10000, -50 });

    m_renderContext = std::make_unique<render_gl::RenderContext>();
    m_renderContext->camera = m_camera;

    m_checkerGridRenderer = std::make_shared<render_gl::CheckerGridRenderer>();
    m_onRenderActions.push_back(
      std::bind(&render_gl::CheckerGridRenderer::render, m_checkerGridRenderer, _1));

    m_rtsaRenderer = std::make_shared<render_gl::RtsaRenderer>(m_container);
    m_onRenderActions.push_back(std::bind(&render_gl::RtsaRenderer::render, m_rtsaRenderer, _1));

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

    // create camera movement controllers

    m_panTool = std::make_unique<PanTool>(m_input, m_renderContext);

    auto cameraBoundsController = std::make_shared<CameraBoundsController>(m_renderContext);
    m_onMainLoopActions.push_back(
      [=, this]()
      {
          const auto rangeX = m_container->getSettings().getMaxFrequency();
          const auto rangeY = -m_container->getSettings().magnitudeDecibelRange;

          const glm::vec3 lowerLeft{ 0, rangeY, 1.0f };
          const glm::vec3 upperRight{ rangeX, 0, 1.0f };

          const auto p1 = lowerLeft;
          const auto p2 = upperRight;

          const auto minX = std::min(p1.x, p2.x);
          const auto maxX = std::max(p1.x, p2.x);
          const auto minY = std::min(p1.y, p2.y);
          const auto maxY = std::max(p1.y, p2.y);

          const glm::vec2 finalLowerLeft{ minX, minY };
          const glm::vec2 finalUpperRight{ maxX, maxY };

          cameraBoundsController->setBounds(finalLowerLeft, finalUpperRight);
          cameraBoundsController->update();
      });

    // view settings widget
    RtsaViewSettingsWidgetSettings rtsaViewSettingsCallbacks{
        .onFrequencyAxisScaleChanged =
          [=, this](const render_gl::FrequencyAxisScale& frequencyAxisScale)
        {
            // TODO implement
            // m_rtsaRenderer->setFrequencyAxisScale(frequencyAxisScale);
            // recreateAxis();
        }
    };

    m_rtsaViewSettingsWidget =
      std::make_shared<RtsaViewSettingsWidget>(uiFont, std::move(rtsaViewSettingsCallbacks));
    m_onRenderActions.push_back(
      std::bind(&RtsaViewSettingsWidget::render, m_rtsaViewSettingsWidget, _1));

    m_rtsaViewSettingsWidget->setFrequencyAxisScale(render_gl::FrequencyAxisScale::Logarithmic);

    // widget showing info about the point where mouse points
    auto worldPointToTextCallback = [=, this](const glm::vec2& worldPoint)
    {
        float frequencyCoord = worldPoint.x;
        float dbfs = worldPoint.y;

        const auto& settings = m_container->getSettings();
        if (m_rtsaViewSettingsWidget->getFrequencyAxisScale() ==
            render_gl::FrequencyAxisScale::Linear)
        {
            // ss << frequencyCoord << " Hz\n";
        }
        else
        {
            /*const auto maxFrequency = m_container->getRangeY().max;
            const auto frequency = render_gl::LogScaleUtils::getFrequency(frequencyCoord,
            maxFrequency); ss << frequency << " Hz\n";*/
        }

        std::stringstream ss;
        ss << fmt::format("Frequency: {}", frequencyCoord) << std::endl;
        ss << fmt::format("dBFS: {}", dbfs) << std::endl;
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
              /*m_rtsaViewSettingsWidget->setShowCursorInfo(
                !m_rtsaViewSettingsWidget->getShowCursorInfo());*/
          }
      });
}

void RtsaWindow::onMainLoopUpdate()
{
    Window::onMainLoopUpdate();

    m_renderContext->viewportSize = { getWidth(), getHeight() };

    for (auto& action : m_onMainLoopActions)
    {
        action();
    }
}

void RtsaWindow::onBeforeRender()
{
    Window::onBeforeRender();
}

void RtsaWindow::onRender()
{
    Window::onRender();

    for (auto& action : m_onRenderActions)
    {
        action(*m_renderContext);
    }
}

void RtsaWindow::onAfterRender()
{
    Window::onAfterRender();

    m_panTool->update();
}

void RtsaWindow::recreateAxis()
{
    m_axes.clear();

    const glm::vec2 magnitudeAxisDirection{ 0, 1 };
    const glm::vec2 frequencyAxisDirection{ 1, 0 };

    const auto magnitudeAxisDock1 = render_gl::AxisDockPosition::Left;
    const auto magnitudeAxisDock2 = render_gl::AxisDockPosition::Right;

    const auto frequencyAxisDock1 = render_gl::AxisDockPosition::Bottom;
    const auto frequencyAxisDock2 = render_gl::AxisDockPosition::Top;

    auto frequencyAxisBottom = std::make_shared<render_gl::AxisRenderer>(
      frequencyAxisDirection, frequencyAxisDock1, m_axisFont);
    auto frequencyAxisBottomDetailed = std::make_shared<render_gl::AxisRenderer>(
      frequencyAxisDirection, frequencyAxisDock1, m_axisFont, 1);

    auto frequencyAxisTop = std::make_shared<render_gl::AxisRenderer>(
      frequencyAxisDirection, frequencyAxisDock2, m_axisFont);
    auto frequencyAxisTopDetailed = std::make_shared<render_gl::AxisRenderer>(
      frequencyAxisDirection, frequencyAxisDock2, m_axisFont, 1);

    auto magnitudeAxisLeft = std::make_shared<render_gl::AxisRenderer>(
      magnitudeAxisDirection, magnitudeAxisDock1, m_axisFont);
    auto magnitudeAxisLeftDetailed = std::make_shared<render_gl::AxisRenderer>(
      magnitudeAxisDirection, magnitudeAxisDock1, m_axisFont, 1);

    auto magnitudeAxisRight = std::make_shared<render_gl::AxisRenderer>(
      magnitudeAxisDirection, magnitudeAxisDock2, m_axisFont);
    auto magnitudeAxisRightDetailed = std::make_shared<render_gl::AxisRenderer>(
      magnitudeAxisDirection, magnitudeAxisDock2, m_axisFont, 1);

    frequencyAxisBottom->setLabelSuffix(" Hz");
    frequencyAxisTop->setLabelSuffix(" Hz");

    magnitudeAxisLeft->setLabelSuffix(" dB");
    magnitudeAxisRight->setLabelSuffix(" dB");

    frequencyAxisBottomDetailed->setIsRenderLabels(false);
    frequencyAxisTopDetailed->setIsRenderLabels(false);

    magnitudeAxisLeftDetailed->setIsRenderLabels(false);
    magnitudeAxisRightDetailed->setIsRenderLabels(false);

    const auto detailedMarkHeightCoeff = 0.5f;

    frequencyAxisBottomDetailed->setMarkHeight(frequencyAxisBottom->getMarkHeight() *
                                               detailedMarkHeightCoeff);

    frequencyAxisTopDetailed->setMarkHeight(frequencyAxisTop->getMarkHeight() *
                                            detailedMarkHeightCoeff);

    magnitudeAxisLeftDetailed->setMarkHeight(magnitudeAxisLeft->getMarkHeight() *
                                             detailedMarkHeightCoeff);

    magnitudeAxisRightDetailed->setMarkHeight(magnitudeAxisRight->getMarkHeight() *
                                              detailedMarkHeightCoeff);

    m_axes.push_back(frequencyAxisBottom);
    m_axes.push_back(frequencyAxisBottomDetailed);

    m_axes.push_back(frequencyAxisTop);
    m_axes.push_back(frequencyAxisTopDetailed);

    m_axes.push_back(magnitudeAxisLeft);
    m_axes.push_back(magnitudeAxisLeftDetailed);

    m_axes.push_back(magnitudeAxisRight);
    m_axes.push_back(magnitudeAxisRightDetailed);
}
}
