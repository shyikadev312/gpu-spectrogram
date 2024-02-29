#include <spectr/desktop_app/HeatmapCursorInfo.h>

#include <spectr/render_gl/ImguiUtils.h>

namespace spectr::desktop_app
{
HeatmapCursorInfo::HeatmapCursorInfo(std::shared_ptr<Input> input,
                                     ImFont* font,
                                     WorldPointTextCallback textCallback)
  : m_input{ input }
  , m_font{ font }
  , m_textCallback{ std::move(textCallback) }
{
}

void HeatmapCursorInfo::onRender(const render_gl::RenderContext& renderContext)
{
    if (!m_isEnabled)
    {
        return;
    }

    if (render_gl::ImguiUtils::isCursorOverUI())
    {
        return;
    }

    const auto cursorCoordinates = m_input->getCursorPosition();

    ImGui::PushFont(m_font);

    const auto windowPivotPoint = ImVec2{ 1.1f, 1.1f };

    const auto viewport = ImGui::GetMainViewport();
    const auto viewportSize = viewport->WorkSize;
    const auto windowPosition = ImVec2{ cursorCoordinates.x, viewportSize.y - cursorCoordinates.y };

    const glm::ivec2 pixelCoordinates(cursorCoordinates.x, cursorCoordinates.y);
    const auto worldPoint = renderContext.pixelToWorld(pixelCoordinates);

    const auto windowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings |
                             ImGuiWindowFlags_AlwaysAutoResize |
                             ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav |
                             ImGuiWindowFlags_NoMove;

    ImGui::SetNextWindowPos(windowPosition, ImGuiCond_Always, windowPivotPoint);
    ImGui::SetNextWindowBgAlpha(0.8f);
    ImGui::Begin("Heatmap info:", nullptr, windowFlags);
    const auto text = m_textCallback(worldPoint);
    ImGui::Text(text.c_str());
    ImGui::End();

    ImGui::PopFont();
}

bool HeatmapCursorInfo::isEnabled() const
{
    return m_isEnabled;
}

void HeatmapCursorInfo::setEnabled(bool isEnabled)
{
    m_isEnabled = isEnabled;
}
}
