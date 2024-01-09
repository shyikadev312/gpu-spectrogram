#include <spectr/render_gl/PanTool.h>

namespace spectr::render_gl
{
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
        diff.y = 0;
    }
    if (m_isShiftPressed)
    {
        diff.x = 0;
    }

    auto newScroll = yoffset > 0 ? currentScroll * diff : currentScroll / diff;

    camera->setScale(newScroll);
}

void PanTool::onMouseButton(GLFWwindow* window, int button, int action, int mods)
{
    //
}

void PanTool::onCursorPosition(GLFWwindow* window, double xpos, double ypos)
{
    //
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
