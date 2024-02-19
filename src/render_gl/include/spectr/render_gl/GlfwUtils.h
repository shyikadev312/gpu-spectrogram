#pragma once

#include <spectr/render_gl/GraphicsApi.h>

namespace spectr::render_gl
{
class GlfwWrapper
{
public:
    GlfwWrapper();

    ~GlfwWrapper();
};

class GlfwWindowWrapper
{
public:
    GlfwWindowWrapper() = default;

    GlfwWindowWrapper(int width,
                      int height,
                      const char* title,
                      GLFWmonitor* monitor,
                      GLFWwindow* share);

    GlfwWindowWrapper(GlfwWindowWrapper&&);
    GlfwWindowWrapper& operator=(GlfwWindowWrapper&&);

    GlfwWindowWrapper(const GlfwWindowWrapper&) = delete;
    GlfwWindowWrapper& operator=(const GlfwWindowWrapper&) = delete;

    ~GlfwWindowWrapper();

    operator GLFWwindow*();

    GLFWwindow* getWindow();

private:
    GLFWwindow* m_window = nullptr;
};
}
