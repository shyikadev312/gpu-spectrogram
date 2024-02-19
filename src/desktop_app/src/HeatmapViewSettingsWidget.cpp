#include <spectr/desktop_app/HeatmapViewSettingsWidget.h>

#include <stdexcept>

namespace spectr::desktop_app
{
namespace
{
const std::vector<render_gl::MagnitudeColorMode> MagnitudeColorModes{
    render_gl::MagnitudeColorMode::Grayscale,
    render_gl::MagnitudeColorMode::FiveColorScale,
};

const std::vector<render_gl::MagnitudeAxisScale> MagnitudeAxisScales{
    render_gl::MagnitudeAxisScale::Linear,
    render_gl::MagnitudeAxisScale::Logarithmic,
};

const std::vector<render_gl::FrequencyAxisScale> FrequencyAxisScales{
    render_gl::FrequencyAxisScale::Linear,
    render_gl::FrequencyAxisScale::Logarithmic,
};

const std::vector<render_gl::MagnitudeInterpolationMode> MagnitudeInterpolationModes{
    render_gl::MagnitudeInterpolationMode::Sharp,
    render_gl::MagnitudeInterpolationMode::Bilinear,
};

const std::vector<render_gl::WaterfallDirection> WaterfallDirections{
    render_gl::WaterfallDirection::Horizontal,
    render_gl::WaterfallDirection::Vertical,
};

template<typename T>
void renderComboBox(const std::string& title,
                    const std::vector<T>& options,
                    T& selectedOption,
                    std::function<void(T)>& onChangeCallback)
{
    ImGui::Text(title.c_str());
    const auto selectedItemStr = render_gl::toString(selectedOption);
    const auto elementId = "##" + title;
    if (ImGui::BeginCombo(
          elementId.c_str(), selectedItemStr.c_str(), ImGuiComboFlags_PopupAlignLeft))
    {
        for (const auto option : options)
        {
            const auto label = render_gl::toString(option);
            const auto isSelected = option == selectedOption;
            if (ImGui::Selectable(label.c_str(), isSelected))
            {
                selectedOption = option;
                onChangeCallback(selectedOption);
            }
        }
        ImGui::EndCombo();
    }
}

template<typename T>
void set(T& currentOption, T newOption, std::function<void(T)>& onChangeCallback)
{
    // if (currentOption != newOption)
    {
        currentOption = newOption;
        onChangeCallback(currentOption);
    }
}
}

HeatmapViewSettingsWidget::HeatmapViewSettingsWidget(ImFont* font,
                                                     HeatmapViewSettingsCallbacks callbacks)
  : m_font{ font }
  , m_callbacks{ std::move(callbacks) }
{
}

void HeatmapViewSettingsWidget::render(const render_gl::RenderContext& renderContext)
{
    ImGui::PushFont(m_font);
    ImGui::Begin("Heatmap view settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    renderComboBox("Magnitude color mode:",
                   MagnitudeColorModes,
                   m_magnitudeColorMode,
                   m_callbacks.onMagnitudeColorModeChanged);

    renderComboBox("Frequency axis scale:",
                   FrequencyAxisScales,
                   m_frequencyAxisScale,
                   m_callbacks.onFrequencyAxisScaleChanged);

    renderComboBox("Magnitude axis scale:",
                   MagnitudeAxisScales,
                   m_magnitudeAxisScale,
                   m_callbacks.onMagnitudeAxisChanged);

    renderComboBox("Magnitude interpolation mode:",
                   MagnitudeInterpolationModes,
                   m_magnitudeInterpolationMode,
                   m_callbacks.onMagnitudeInterpolationModeChanged);

    renderComboBox("Waterfall direction mode:",
                   WaterfallDirections,
                   m_waterfallDirection,
                   m_callbacks.onWaterfallDirectionChanged);

    bool isShowCursorInfo = m_isShowCursorInfo;
    ImGui::Checkbox("Mouse pointer info (toggle by Q):", &isShowCursorInfo);
    if (m_isShowCursorInfo != isShowCursorInfo)
    {
        setShowCursorInfo(isShowCursorInfo);
    }

    ImGui::End();
    ImGui::PopFont();
}

render_gl::MagnitudeColorMode HeatmapViewSettingsWidget::getMagnitudeColorMode() const
{
    return m_magnitudeColorMode;
}

void HeatmapViewSettingsWidget::setMagnitudeColorMode(
  render_gl::MagnitudeColorMode magnitudeColorMode)
{
    set(m_magnitudeColorMode, magnitudeColorMode, m_callbacks.onMagnitudeColorModeChanged);
}

render_gl::FrequencyAxisScale HeatmapViewSettingsWidget::getFrequencyAxisScale() const
{
    return m_frequencyAxisScale;
}

void HeatmapViewSettingsWidget::setFrequencyAxisScale(
  render_gl::FrequencyAxisScale frequencyAxisScale)
{
    set(m_frequencyAxisScale, frequencyAxisScale, m_callbacks.onFrequencyAxisScaleChanged);
}

render_gl::MagnitudeAxisScale HeatmapViewSettingsWidget::getMagnitudeAxisScale() const
{
    return m_magnitudeAxisScale;
}

void HeatmapViewSettingsWidget::setMagnitudeAxisScale(
  render_gl::MagnitudeAxisScale magnitudeAxisScale)
{
    set(m_magnitudeAxisScale, magnitudeAxisScale, m_callbacks.onMagnitudeAxisChanged);
}

render_gl::MagnitudeInterpolationMode HeatmapViewSettingsWidget::getMagnitudeInterpolationMode()
  const
{
    return m_magnitudeInterpolationMode;
}

void HeatmapViewSettingsWidget::setMagnitudeInterpolationMode(
  render_gl::MagnitudeInterpolationMode magnitudeInterpolationMode)
{
    set(m_magnitudeInterpolationMode,
        magnitudeInterpolationMode,
        m_callbacks.onMagnitudeInterpolationModeChanged);
}

render_gl::WaterfallDirection HeatmapViewSettingsWidget::getWaterfallDirection() const
{
    return m_waterfallDirection;
}

void HeatmapViewSettingsWidget::setWaterfallDirection(
  render_gl::WaterfallDirection waterfallDirection)
{
    set(m_waterfallDirection, waterfallDirection, m_callbacks.onWaterfallDirectionChanged);
}

bool HeatmapViewSettingsWidget::getShowCursorInfo() const
{
    return m_isShowCursorInfo;
}

void HeatmapViewSettingsWidget::setShowCursorInfo(bool isShowCursorInfo)
{
    m_isShowCursorInfo = isShowCursorInfo;
    m_callbacks.onShowCursorInfoChanged(m_isShowCursorInfo);
}
}
