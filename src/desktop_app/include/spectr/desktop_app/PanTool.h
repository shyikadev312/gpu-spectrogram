#pragma once

#include <spectr/desktop_app/Input.h>
#include <spectr/render_gl/GraphicsApi.h>
#include <spectr/render_gl/RenderContext.h>

#include <chrono>
#include <memory>

namespace spectr::desktop_app
{
// TODO move to another module?
class PanTool
{
public:
    PanTool(std::shared_ptr<Input> input, std::shared_ptr<render_gl::RenderContext> renderContext);

    void update();

    void checkKeyboard();

    void checkScroll();

    void checkMouseButton();

    void checkCursorPosition();

    void setIsEnabled(bool isEnabled);

private:
    bool m_isEnabled = true;
    std::shared_ptr<Input> m_input;
    std::shared_ptr<render_gl::RenderContext> m_renderContext;
    float m_arrowMoveSpeed = 3.0f;
    std::chrono::high_resolution_clock::time_point m_prevFrameTime;
    glm::vec2 m_direction{};

    bool m_isControlPressed = false;
    bool m_isShiftPressed = false;
    glm::vec2 m_scrollSpeedCoeff = { 1.05f, 1.05f };

    bool m_mouseWasPressed = false;
    bool m_isMouseMoving = false;
    glm::vec2 m_mouseStartPosition{};
    glm::vec2 m_mouseMoveCameraStartPosition{};

    bool m_isCursorOverUI = false;
};
}
