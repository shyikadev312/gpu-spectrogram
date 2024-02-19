#include <spectr/desktop_app/PanTool.h>

#include <spectr/render_gl/ImguiUtils.h>

namespace spectr::desktop_app
{
PanTool::PanTool(std::shared_ptr<Input> input,
                 std::shared_ptr<render_gl::RenderContext> renderContext)
  : m_input{ input }
  , m_renderContext{ renderContext }
{
}

void PanTool::update()
{
    const auto now = std::chrono::high_resolution_clock::now();
    const auto timeDelta = std::chrono::duration<float>(now - m_prevFrameTime).count();
    m_prevFrameTime = now;

    if (!m_isEnabled)
    {
        return;
    }

    checkKeyboard();
    checkScroll();
    checkMouseButton();
    checkCursorPosition();

    m_isCursorOverUI = render_gl::ImguiUtils::isCursorOverUI();

    if (glm::length(m_direction) > 0.1f)
    {
        m_direction = glm::normalize(m_direction);
        auto& camera = *m_renderContext->camera;
        const auto oldPos = camera.getPositionWorld() * camera.getScale();
        const auto newPos = oldPos + timeDelta * m_arrowMoveSpeed * m_direction;
        camera.setPositionWorld(newPos / camera.getScale());
    }
}

void PanTool::checkKeyboard()
{
    m_direction = {};

    if (m_input->isKeyPressed(GLFW_KEY_UP))
    {
        m_direction.y = 1;
    }
    if (m_input->isKeyPressed(GLFW_KEY_DOWN))
    {
        m_direction.y = -1;
    }
    if (m_input->isKeyPressed(GLFW_KEY_LEFT))
    {
        m_direction.x = -1;
    }
    if (m_input->isKeyPressed(GLFW_KEY_RIGHT))
    {
        m_direction.x = 1;
    }
}

void PanTool::checkScroll()
{
    m_isControlPressed =
      m_input->isKeyPressed(GLFW_KEY_LEFT_CONTROL) || m_input->isKeyPressed(GLFW_KEY_RIGHT_CONTROL);

    m_isShiftPressed =
      m_input->isKeyPressed(GLFW_KEY_LEFT_SHIFT) || m_input->isKeyPressed(GLFW_KEY_RIGHT_SHIFT);

    const auto yoffset = m_input->getMouseScrollDelta().y;

    if (std::abs(yoffset) < 1e-3)
    {
        return;
    }

    auto camera = m_renderContext->camera;
    const auto currentScroll = camera->getScale();
    auto diff = glm::vec2{ std::abs(yoffset), std::abs(yoffset) } * m_scrollSpeedCoeff;

    if (m_isControlPressed)
    {
        diff.y = 1;
    }
    if (m_isShiftPressed)
    {
        diff.x = 1;
    }

    glm::ivec2 cursorPositionPixel = m_input->getCursorPosition();

    const auto focusBefore = m_renderContext->pixelToWorld(cursorPositionPixel);

    auto newScroll = yoffset > 0 ? currentScroll * diff : currentScroll / diff;
    camera->setScale(newScroll);

    const auto focusAfter = m_renderContext->pixelToWorld(cursorPositionPixel);
    const auto cameraPositionDiff = focusBefore - focusAfter;

    camera->setPositionWorld(camera->getPositionWorld() + cameraPositionDiff);
}

void PanTool::checkMouseButton()
{
    if (m_isCursorOverUI)
    {
        return;
    }

    const auto isPressed = m_input->isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT);
    m_isMouseMoving = isPressed;

    if (isPressed && !m_mouseWasPressed)
    {
        m_mouseMoveCameraStartPosition = m_renderContext->camera->getPositionWorld();
        m_mouseStartPosition = m_input->getCursorPosition();
    }

    m_mouseWasPressed = isPressed;
}

void PanTool::checkCursorPosition()
{
    if (m_isMouseMoving)
    {
        const auto currentCursorPosition = m_input->getCursorPosition();
        const auto pixelsDiff = currentCursorPosition - m_mouseStartPosition;
        const auto w1 = m_renderContext->pixelToWorld(m_mouseStartPosition);
        const auto w2 = m_renderContext->pixelToWorld(currentCursorPosition);
        const auto worldDiff = w1 - w2;
        m_renderContext->camera->setPositionWorld(m_mouseMoveCameraStartPosition + worldDiff);
    }
}

void PanTool::setIsEnabled(bool isEnabled)
{
    m_isEnabled = isEnabled;
}
}
