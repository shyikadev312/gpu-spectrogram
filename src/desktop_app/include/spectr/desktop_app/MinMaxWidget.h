#pragma once

#include <spectr/render_gl/RenderContext.h>

namespace spectr::desktop_app
{
class MinMaxWidget
{
public:
    MinMaxWidget();

    void setRange(float globalMin, float globalMax);

    void render(const render_gl::RenderContext& renderContext);

    float getMin() const;

    float getMax() const;

private:
    float m_globalMin = 0;
    float m_globalMax = 1;
    float m_currentMin = 0;
    float m_currentMax = 1;
};
}
