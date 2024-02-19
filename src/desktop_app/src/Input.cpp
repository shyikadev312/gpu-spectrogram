#include <spectr/desktop_app/Input.h>

namespace spectr::desktop_app
{
Input* getInput(GLFWwindow* window)
{
    return reinterpret_cast<Input*>(glfwGetWindowUserPointer(window));
}

void keyCallbackGlobal(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    auto input = getInput(window);
    input->keyCallback(window, key, scancode, action, mods);
}

void scrollCallbackGlobal(GLFWwindow* window, double xoffset, double yoffset)
{
    auto input = getInput(window);
    input->scrollCallback(window, xoffset, yoffset);
}

void mouseButtonCallbackGlobal(GLFWwindow* window, int button, int action, int mods)
{
    auto input = getInput(window);
    input->mouseButtonCallback(window, button, action, mods);
}

void cursorPositionCallbackGlobal(GLFWwindow* window, double xpos, double ypos)
{
    auto input = getInput(window);
    input->cursorPositionCallback(window, xpos, ypos);
}

Input::Input(GLFWwindow* window)
  : m_window{ window }
{
    glfwSetWindowUserPointer(window, this);
    glfwSetKeyCallback(window, keyCallbackGlobal);
    glfwSetScrollCallback(window, scrollCallbackGlobal);
    glfwSetMouseButtonCallback(window, mouseButtonCallbackGlobal);
    glfwSetCursorPosCallback(window, cursorPositionCallbackGlobal);
}

void Input::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    const auto isPressed = action == GLFW_PRESS || action == GLFW_REPEAT;

    ButtonData data;
    if (m_keyStates.contains(key))
    {
        data = m_keyStates[key];
        if (data.isPressed != isPressed)
        {
            data.isPressed = isPressed;
            data.isChangedThisFrame = true;
        }
    }
    else
    {
        data.isPressed = isPressed;
        data.isChangedThisFrame = true;
    }

    m_keyStates[key] = data;
}

void Input::scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    m_mouseWheelScrollDelta = { xoffset, yoffset };
}

void Input::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    m_mouseButtonStates[button] = action;
}

void Input::cursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
{
    m_cursorPosition = { xpos, ypos };
}

void Input::newFrame()
{
    for (auto& [key, state] : m_keyStates)
    {
        state.isChangedThisFrame = false;
    }

    m_mouseWheelScrollDelta = {};
}

bool Input::isKeyDown(int key) const
{
    const auto& state = m_keyStates[key];
    return state.isPressed && state.isChangedThisFrame;
}

bool Input::isKeyPressed(int key) const
{
    const auto& state = m_keyStates[key];
    return state.isPressed;
}

bool Input::isMouseButtonPressed(int button) const
{
    const auto state = m_mouseButtonStates[button];
    return state == GLFW_PRESS || state == GLFW_REPEAT;
}

glm::vec2 Input::getCursorPosition() const
{
    // TODO change: screen coords and pixel coords are not the same
    int height;
    glfwGetFramebufferSize(m_window, nullptr, &height);
    glm::vec2 pos = m_cursorPosition;
    pos.y = height - pos.y;
    return { static_cast<int>(pos.x), static_cast<int>(pos.y) };
}

glm::vec2 Input::getMouseScrollDelta() const
{
    return m_mouseWheelScrollDelta;
}
}
