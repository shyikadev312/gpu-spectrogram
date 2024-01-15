#include <spectr/render_gl/PanTool.h>

namespace spectr::render_gl
{
namespace
{
glm::ivec2 getCursorPosition(GLFWwindow* window, const RenderContext& renderContext)
{
    double xpos = 0;
    double ypos = 0;
    glfwGetCursorPos(window, &xpos, &ypos);
    ypos = renderContext.viewportSize.y - ypos;
    return { static_cast<int>(xpos), static_cast<int>(ypos) };
}
}

PanTool::PanTool(std::shared_ptr<render_gl::RenderContext> renderContext)
  : m_renderContext{ renderContext }
{
}

void PanTool::onKeyboardInput(int key, int scancode, int action, int mods)
{
    m_isControlPressed = (key == GLFW_KEY_LEFT_CONTROL || key == GLFW_KEY_RIGHT_CONTROL) &&
                         (action == GLFW_PRESS || action == GLFW_REPEAT);

    m_isShiftPressed = (key == GLFW_KEY_LEFT_SHIFT || key == GLFW_KEY_RIGHT_SHIFT) &&
                       (action == GLFW_PRESS || action == GLFW_REPEAT);

    if (action == GLFW_PRESS)
    {
        switch (key)
        {
            case GLFW_KEY_UP:
            {
                m_direction.y = 1;
                break;
            }
            case GLFW_KEY_DOWN:
            {
                m_direction.y = -1;
                break;
            }
            case GLFW_KEY_LEFT:
            {
                m_direction.x = -1;
                break;
            }
            case GLFW_KEY_RIGHT:
            {
                m_direction.x = 1;
                break;
            }
        }
    }

    if (action == GLFW_RELEASE)
    {
        switch (key)
        {
            case GLFW_KEY_UP:
            {
                m_direction.y = 0;
                break;
            }
            case GLFW_KEY_DOWN:
            {
                m_direction.y = 0;
                break;
            }
            case GLFW_KEY_LEFT:
            {
                m_direction.x = 0;
                break;
            }
            case GLFW_KEY_RIGHT:
            {
                m_direction.x = 0;
                break;
            }
        }
    }
}

void PanTool::onScroll(GLFWwindow* window, double xoffset, double yoffset)
{
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

    glm::ivec2 cursorPositionPixel = getCursorPosition(window, *m_renderContext);

    const auto focusBefore = m_renderContext->pixelToWorld(cursorPositionPixel);

    auto newScroll = yoffset > 0 ? currentScroll * diff : currentScroll / diff;
    camera->setScale(newScroll);

    const auto focusAfter = m_renderContext->pixelToWorld(cursorPositionPixel);
    const auto scaledCameraPositionDiff = (focusBefore - focusAfter) * camera->getScale();

    camera->setPosition(camera->getPosition() + scaledCameraPositionDiff);
}

void PanTool::onMouseButton(GLFWwindow* window, int button, int action, int mods)
{
    // if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    //{
    //     auto& camera = *m_renderContext->camera;
    //     const auto curPos = m_renderContext->camera->getPosition();
    //     camera.setPosition(curPos + glm::vec2(10, 0) * camera.getScale());

    //    auto pos2 = camera.getPosition();
    //}

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        m_mouseMoveCameraStartPosition = m_renderContext->camera->getPosition();
        m_mouseStartPosition = getCursorPosition(window, *m_renderContext);
        m_isMouseMoving = true;
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        m_isMouseMoving = false;
    }
}

void PanTool::onCursorPosition(GLFWwindow* window, double xpos, double ypos)
{
    if (m_isMouseMoving)
    {
        const auto currentCursorPosition = getCursorPosition(window, *m_renderContext);
        const auto pixelsDiff = currentCursorPosition - m_mouseStartPosition;
        const auto w1 = m_renderContext->pixelToWorld(m_mouseStartPosition);
        const auto w2 = m_renderContext->pixelToWorld(currentCursorPosition);
        const auto worldDiff = (w1 - w2) * m_renderContext->camera->getScale();
        m_renderContext->camera->setPosition(m_mouseMoveCameraStartPosition + worldDiff);
    }
}

void PanTool::onFrame()
{
    if (glm::length(m_direction) > 0.1f)
    {
        m_direction = glm::normalize(m_direction);

        auto& camera = *m_renderContext->camera;
        const auto oldPos = camera.getPosition();

        auto now = std::chrono::high_resolution_clock::now();
        auto diff = now - m_prevFrameTime;
        std::chrono::duration<float> timeDiff = now - m_prevFrameTime;
        const auto timeDelta = timeDiff.count();
        // std::cout << "Elapsed Time: " << timeDelta.count() << " seconds" << std::endl;

        const auto newPos = oldPos + timeDelta * m_arrowMoveSpeed * m_direction;
        camera.setPosition(newPos);
    }

    m_prevFrameTime = std::chrono::high_resolution_clock::now();
}
}
