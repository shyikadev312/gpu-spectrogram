#pragma once

#include <spectr/render_gl/GraphicsApi.h>
#include <spectr/render_gl/RenderContext.h>

#include <chrono>
#include <memory>

namespace spectr::render_gl
{
// TODO move to another module?
class PanTool
{
public:
    PanTool(std::shared_ptr<render_gl::RenderContext> renderContext);

    void onKeyboardInput(int key, int scancode, int action, int mods);

    void onScroll(GLFWwindow* window, double xoffset, double yoffset);

    void onMouseButton(GLFWwindow* window, int button, int action, int mods);

    void onCursorPosition(GLFWwindow* window, double xpos, double ypos);

    void onFrame();

private:
    std::shared_ptr<render_gl::RenderContext> m_renderContext;
    float m_arrowMoveSpeed = 3.0f;
    std::chrono::high_resolution_clock::time_point m_prevFrameTime;
    glm::vec2 m_direction{};

    bool m_isControlPressed = false;
    bool m_isShiftPressed = false;
    glm::vec2 m_scrollSpeedCoeff = { 1.05f, 1.05f };

    bool m_isMouseMoving = false;
    glm::ivec2 m_mouseStartPosition{};
    glm::vec2 m_mouseMoveCameraStartPosition{};
};
}
