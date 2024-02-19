#pragma once

#include <spectr/render_gl/GraphicsApi.h>

#include <unordered_map>

namespace spectr::desktop_app
{
class Input
{
public:
    // Global callbacks for GLFW:

    friend void keyCallbackGlobal(GLFWwindow* window, int key, int scancode, int action, int mods);

    friend void scrollCallbackGlobal(GLFWwindow* window, double xoffset, double yoffset);

    friend void mouseButtonCallbackGlobal(GLFWwindow* window, int button, int action, int mods);

    friend void cursorPositionCallbackGlobal(GLFWwindow* window, double xpos, double ypos);

    Input(GLFWwindow* window);

    // Local callbacks for Input instance:

    void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

    void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

    void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

    void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos);

    void newFrame();

    // User-scope functions for input

    bool isKeyDown(int key) const;

    bool isKeyPressed(int key) const;

    // bool isKeyUp(int key) const;

    // bool isMouseButtonDown(int button) const;

    bool isMouseButtonPressed(int button) const;

    // bool isMouseButtonUp(int button) const;

    glm::vec2 getCursorClipSpaceCoordinates();

    glm::vec2 getCursorPosition() const;

    glm::vec2 getMouseScrollDelta() const;

private:
    struct ButtonData
    {
        bool isPressed;
        bool isChangedThisFrame;
    };

    GLFWwindow* m_window = nullptr;
    glm::vec2 m_cursorPosition = {};
    glm::vec2 m_mouseWheelScrollDelta = {};
    mutable std::unordered_map<int, int> m_mouseButtonStates;
    mutable std::unordered_map<int, ButtonData> m_keyStates;
};
}
