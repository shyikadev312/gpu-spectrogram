#pragma once

#include <spectr/render_gl/GraphicsApi.h>
#include <spectr/render_gl/RenderContext.h>

#include <string>

namespace spectr::render_gl
{
enum class AxisRenderMode
{
    Vertical,
    Horizontal
};

class AxisRendererSettings
{
public:
    AxisRenderMode mode;
    std::string suffix;
};

class AxisRenderer
{
public:
    AxisRenderer(AxisRenderMode mode);

    ~AxisRenderer();

    void render(const RenderContext& renderContext);

    bool getIsEnabled() const;

    void setIsEnabled(bool isEnabled);

    bool getIsRenderLabels() const;

    void setIsRenderLabels(bool isRenderLabels);

    bool getIsRenderMarks() const;

    void setIsRenderMarks(bool isRenderMarks);

    float getMarksStep() const;

    void setMarksStep(float marksStep);

private:
private:
    AxisRenderMode m_mode;
    /*glm::vec2 m_ndcMin;
    glm::vec2 m_ndcMax;*/
    ImFont* m_font;
    bool m_isEnabled = true;
    bool m_isRenderLabels = true;
    bool m_isRenderMarks = true;
    float m_marksStep = 1.0f;
    float m_markWidth = 5.0f;
    float m_markHeight = 30.0f;
};
}
