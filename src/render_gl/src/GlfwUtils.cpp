#include <spectr/render_gl/GlfwUtils.h>

#include <stdexcept>

namespace spectr::render_gl
{
GlfwWrapper::GlfwWrapper()
{
    if (!glfwInit())
    {
        throw std::runtime_error("Failed to init GLFW.");
    }
}

GlfwWrapper::~GlfwWrapper()
{
    glfwTerminate();
}

GlfwWindowWrapper::GlfwWindowWrapper(int width,
                                     int height,
                                     const char* title,
                                     GLFWmonitor* monitor,
                                     GLFWwindow* share)
  : m_window{ glfwCreateWindow(width, height, title, monitor, share) }
{
}

GlfwWindowWrapper::GlfwWindowWrapper(GlfwWindowWrapper&& other)
  : m_window{ other.m_window }
{
    other.m_window = nullptr;
}

GlfwWindowWrapper& GlfwWindowWrapper::operator=(GlfwWindowWrapper&& other)
{
    if (m_window)
    {
        glfwDestroyWindow(m_window);
    }

    m_window = other.m_window;
    other.m_window = nullptr;

    return *this;
}

GlfwWindowWrapper::~GlfwWindowWrapper()
{
    if (m_window)
    {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
}

GlfwWindowWrapper::operator GLFWwindow*()
{
    return m_window;
}

GLFWwindow* GlfwWindowWrapper::getWindow()
{
    return m_window;
}
}
