#include <spectr/render_gl/FpsGuard.h>

#include <spectr/render_gl/ImguiUtils.h>

#include <fmt/format.h>

#include <thread>

namespace spectr::render_gl
{
FpsGuard::FpsGuard(size_t targetFps)
  : m_targetFps{ targetFps }
  , m_lastFrameTime{ std::chrono::high_resolution_clock::now() }
  , m_lastFrameDuration{ 0 }
{
}

void FpsGuard::onFrameStart() {}

void FpsGuard::onRender()
{
    ImGui::Begin("FPS info");
    const auto seconds =
      std::chrono::duration_cast<std::chrono::duration<float>>(m_lastFrameDuration);
    const auto text = fmt::format("Frame duration: {:.3f} s.", seconds.count());
    ImGui::Text(text.c_str());
    // ImGui::Text("FPS: ");
    ImGui::End();
}

void FpsGuard::onFrameEnd()
{
    const auto now = std::chrono::high_resolution_clock::now();
    m_lastFrameDuration = now - m_lastFrameTime;
    m_lastFrameTime = std::chrono::high_resolution_clock::now();

    // const auto sleepTime = 10;
    // std::this_thread::sleep_for(sleepTime);
}
}
