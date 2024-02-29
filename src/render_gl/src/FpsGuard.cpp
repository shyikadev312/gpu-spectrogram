#include <spectr/render_gl/FpsGuard.h>

#include <spectr/render_gl/ImguiUtils.h>

#include <format>

#include <thread>

namespace spectr::render_gl
{
FpsGuard::FpsGuard(ImFont* font, size_t targetFps)
  : m_font{ font }
  , m_targetFps{ targetFps }
  , m_lastFrameTime{ std::chrono::high_resolution_clock::now() }
  , m_lastFrameDuration{ 0 }
{
}

void FpsGuard::onFrameStart() {}

void FpsGuard::onRender()
{
    ImGui::PushFont(m_font);
    ImGui::Begin("FPS info");
    const auto seconds = std::chrono::duration<float>(m_lastFrameDuration);
    const auto text = std::format("Frame duration: {:.3f} s.", seconds.count());
    ImGui::Text(text.c_str());
    // ImGui::Text("FPS: ");
    ImGui::End();
    ImGui::PopFont();
}

void FpsGuard::onFrameEnd()
{
    const auto now = std::chrono::high_resolution_clock::now();
    m_lastFrameDuration = now - m_lastFrameTime;
    m_lastFrameTime = std::chrono::high_resolution_clock::now();

    const auto duration = std::chrono::duration<float>(m_lastFrameDuration).count();
    const auto targetDuration = 1.0f / 60.0f;

    const auto diff = targetDuration - duration;
    if (diff > 0)
    {
        std::this_thread::sleep_for(std::chrono::duration<float>(diff));
    }
}
}
