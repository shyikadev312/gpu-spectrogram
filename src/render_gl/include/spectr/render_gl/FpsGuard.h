#pragma once

#include <spectr/render_gl/ImguiUtils.h>

#include <chrono>

namespace spectr::render_gl
{
class FpsGuard
{
public:
    FpsGuard(ImFont* font = nullptr, size_t targetFps = 60);

    void onFrameStart();

    void onRender();

    void onFrameEnd();

private:
    ImFont* m_font = nullptr;
    size_t m_targetFps;
    std::chrono::high_resolution_clock::time_point m_lastFrameTime;
    std::chrono::high_resolution_clock::duration m_lastFrameDuration;
};
}
