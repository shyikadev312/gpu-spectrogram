#pragma once

#include <chrono>

namespace spectr::render_gl
{
class FpsGuard
{
public:
    FpsGuard(size_t targetFps = 60);

    void onFrameStart();

    void onRender();

    void onFrameEnd();

private:
    size_t m_targetFps;
    std::chrono::high_resolution_clock::time_point m_lastFrameTime;
    std::chrono::high_resolution_clock::duration m_lastFrameDuration;
};
}
