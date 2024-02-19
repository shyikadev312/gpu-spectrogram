#include <spectr/desktop_app/RtsaViewSettingsWidget.h>

namespace spectr::desktop_app
{
RtsaViewSettingsWidget::RtsaViewSettingsWidget(ImFont* font,
                                               RtsaViewSettingsWidgetSettings settings)
  : m_font{ font }
  , m_settings{ std::move(settings) }
{
}

void RtsaViewSettingsWidget::render(const render_gl::RenderContext& renderContext)
{
    ImGui::PushFont(m_font);
    ImGui::Begin("Signal density view settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Text("Placeholder.");

    /*renderComboBox("Frequency axis scale:",
                   FrequencyAxisScales,
                   m_frequencyAxisScale,
                   m_callbacks.onFrequencyAxisScaleChanged);*/

    ImGui::End();
    ImGui::PopFont();
}

render_gl::FrequencyAxisScale RtsaViewSettingsWidget::getFrequencyAxisScale() const
{
    return m_frequencyAxisScale;
}

void RtsaViewSettingsWidget::setFrequencyAxisScale(render_gl::FrequencyAxisScale frequencyAxisScale)
{
    m_frequencyAxisScale = frequencyAxisScale;
    m_settings.onFrequencyAxisScaleChanged(frequencyAxisScale);
}
}
