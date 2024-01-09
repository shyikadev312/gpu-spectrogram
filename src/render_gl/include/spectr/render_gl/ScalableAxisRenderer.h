#pragma once

#include <spectr/render_gl/AxisRenderer.h>
#include <spectr/render_gl/GraphicsApi.h>
#include <spectr/render_gl/RenderContext.h>

namespace spectr::render_gl
{
class ScalableAxisRenderer
{
public:
    ScalableAxisRenderer(AxisRenderMode axisRenderMode);

    void render(RenderContext& renderContext);

private:
    AxisRenderMode m_axisMode;
    AxisRenderer m_axis1;
    AxisRenderer m_axis2;
};
}
