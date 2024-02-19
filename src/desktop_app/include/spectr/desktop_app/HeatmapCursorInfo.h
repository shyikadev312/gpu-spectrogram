#pragma once

#include <spectr/desktop_app/Input.h>
#include <spectr/render_gl/GraphicsApi.h>
#include <spectr/render_gl/RenderContext.h>

#include <functional>
#include <string>

namespace spectr::desktop_app
{
using WorldPointTextCallback = std::function<std::string(const glm::vec2&)>;

class HeatmapCursorInfo
{
public:
    HeatmapCursorInfo(std::shared_ptr<Input> input,
                      ImFont* font,
                      WorldPointTextCallback textCallback);

    void onRender(const render_gl::RenderContext& renderContext);

    bool isEnabled() const;

    void setEnabled(bool isEnabled);

private:
    bool m_isEnabled = true;
    std::shared_ptr<Input> m_input;
    ImFont* m_font = nullptr;
    WorldPointTextCallback m_textCallback;
};
}
