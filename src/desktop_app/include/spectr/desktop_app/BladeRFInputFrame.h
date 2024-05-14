#pragma once

#include <spectr/render_gl/RenderContext.h>

namespace spectr::desktop_app {
    class BladeRFInputFrame {
    public:
        BladeRFInputFrame(ImFont* font);

        void render(const render_gl::RenderContext& renderContext);

        size_t getFrequency();

        size_t getSampleRate();

    private:
        ImFont* m_font = nullptr;
        size_t m_frequency = 0;
        size_t m_samplerate = 0;
    };
}
