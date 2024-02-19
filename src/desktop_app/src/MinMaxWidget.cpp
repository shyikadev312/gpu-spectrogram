#include <spectr/desktop_app/MinMaxWidget.h>

#include <spectr/render_gl/GraphicsApi.h>

namespace spectr::desktop_app
{
namespace
{
constexpr float Eps = 1e-5f;
}

MinMaxWidget::MinMaxWidget(ImFont* font)
  : m_font{ font }
{
}

void MinMaxWidget::setRange(float globalMin, float globalMax)
{
    // if mode is "stick to max"
    const bool isStickToMax = std::abs(m_currentMax - m_globalMax) < Eps;

    m_globalMin = globalMin;
    m_globalMax = globalMax;

    if (isStickToMax)
    {
        m_currentMax = m_globalMax;
    }
}

void MinMaxWidget::render(const render_gl::RenderContext& renderContext)
{
    ImGui::PushFont(m_font);

    ImGui::Begin("Min-max spectrogram values:", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("Min");
    ImGui::SameLine();
    ImGui::SliderFloat("##Min", &m_currentMin, m_globalMin, m_globalMax);
    ImGui::Text("Max");
    ImGui::SameLine();
    ImGui::SliderFloat("##Max", &m_currentMax, m_globalMin, m_globalMax);
    ImGui::End();

    if (m_currentMax < m_currentMin)
    {
        m_currentMax = m_currentMin;
    }

    ImGui::PopFont();
}

float MinMaxWidget::getMin() const
{
    return m_currentMin;
}

float MinMaxWidget::getMax() const
{
    return m_currentMax;
}
}
