#pragma once

#include <spectr/render_gl/RenderContext.h>

#include <functional>
#include <string>

namespace spectr::desktop_app
{
extern const std::string UIFontAssetPath;

class Window
{
public:
    virtual ~Window() = default;

    virtual void onMainLoopUpdate();

    virtual void onBeforeRender();

    virtual void onRender();

    virtual void onAfterRender();

    void setSize(int width, int height);

    int getWidth() const;

    int getHeight() const;

protected:
    std::vector<std::function<void()>> m_onMainLoopActions;
    std::vector<std::function<void(const render_gl::RenderContext&)>> m_onRenderActions;

private:
    int m_width = 0;
    int m_height = 0;
};
}
