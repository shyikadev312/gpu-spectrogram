#include <spectr/desktop_app/Window.h>

namespace spectr::desktop_app
{
const std::string UIFontAssetPath =
  "fonts/intel_one/fonts/ttf/intelone-mono-font-family-regular.ttf";

void Window::onMainLoopUpdate() {}

void Window::onBeforeRender() {}

void Window::onRender() {}

void Window::onAfterRender() {}

void Window::setSize(int width, int height)
{
    m_width = width;
    m_height = height;
}

int Window::getWidth() const
{
    return m_width;
}

int Window::getHeight() const
{
    return m_height;
}
}
