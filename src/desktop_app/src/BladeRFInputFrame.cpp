#include <spectr/desktop_app/BladeRFInputFrame.h>
#include <spectr/real_time_input/RealTimeInputBladeRF.h>

namespace spectr::desktop_app {
    bool ImGuiSliderInt64(const char* label, size_t* v, size_t v_min, size_t v_max, const char* format = "%llu", ImGuiSliderFlags flags = 0) {
        return ImGui::SliderScalar(label, ImGuiDataType_U64, v, &v_min, &v_max, format, flags);
    }

    BladeRFInputFrame::BladeRFInputFrame(ImFont* font) : m_font(font) { }

    void BladeRFInputFrame::render(const render_gl::RenderContext& renderContext) {
        ImGui::PushFont(m_font);

        ImGui::Begin("BladeRF settings:", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("Frequency");
        ImGui::SameLine();
        ImGuiSliderInt64("##Frequency", &m_frequency, real_time_input::RealTimeInputBladeRF::min_frequency, real_time_input::RealTimeInputBladeRF::max_frequency);
        ImGui::Text("Sample Rate");
        ImGui::SameLine();
        ImGuiSliderInt64("##SampleRate", &m_samplerate, real_time_input::RealTimeInputBladeRF::min_sample_rate, real_time_input::RealTimeInputBladeRF::max_sample_rate);
        ImGui::End();

        ImGui::PopFont();
    }

    size_t BladeRFInputFrame::getFrequency() {
        return m_frequency;
    }

    size_t BladeRFInputFrame::getSampleRate() {
        return m_samplerate;
    }
}
