#pragma once

#include <spectr/render_gl/GraphicsApi.h>
#include <spectr/render_gl/OpenGlUtils.h>
#include <spectr/render_gl/RenderContext.h>

namespace spectr::render_gl
{
enum class AxisDockPosition
{
    Left,
    Right,
    Top,
    Bottom,
    Center,
};

class AxisRenderer
{
public:
    AxisRenderer(const glm::vec2& axisDirection,
                 AxisDockPosition axisDockPosition,
                 ImFont* font,
                 int detailOrder = 0);

    virtual ~AxisRenderer();

    void render(const RenderContext& renderContext);

    bool getIsEnabled() const;

    void setIsEnabled(bool isEnabled);

    bool getIsRenderLabels() const;

    void setIsRenderLabels(bool isRenderLabels);

    bool getIsRenderMarks() const;

    void setIsRenderMarks(bool isRenderMarks);

    float getMarkHeight() const;

    void setMarkHeight(float markHeightPixels);

    void setLabelSuffix(std::string suffix);

protected:
    virtual float getWorldToAxis(float worldCoordinate) const;

    virtual float getAxisToWorld(float axisCoordinate) const;

private:
    void renderMark(float worldCoordinate,
                    const std::string& text,
                    const RenderContext& renderContext);

    void renderLine(const glm::vec2& lineStart,
                    const glm::vec2& lineEnd,
                    const float lineWidth,
                    const Color& lineColor,
                    const RenderContext& renderContext);

    void renderLabel(const std::string& title,
                     const std::string& text,
                     const glm::ivec2& pixelCoords);

    std::pair<glm::vec2, glm::vec2> getVisibleAxisPixelRange(
      const RenderContext& renderContext) const;

    std::pair<glm::vec2, glm::vec2> getVisibleAxisWorldRange(
      const RenderContext& renderContext) const;

    Range getVisibleAxisCoordinatesRange(const RenderContext& renderContext) const;

    glm::vec2 getWorldPointFromAxisCoordinate(float worldCoordinate,
                                              const RenderContext& renderContext) const;

    bool isHorizontal() const;

    bool isSameDirection() const;

    float getAxisMarkStep(const RenderContext& renderContext) const;

private:
    const glm::vec2 m_axisDirection;
    const AxisDockPosition m_axisDockPosition;
    const bool m_isHorizontal;
    const int m_detailOrder = 0;
    ImFont* m_font = nullptr;
    bool m_isEnabled = true;
    bool m_isRenderLabels = true;
    bool m_isRenderMarks = true;
    float m_markWidth = 5.0f;
    float m_markHeight = 50.0f;
    Color m_markColor = makeColor(133, 96, 54);
    GLuint m_lineShaderProgram = NoShaderProgram;
    GLuint m_lineVbo = NoBuffer;
    GLuint m_lineVao = NoBuffer;
    GLint m_lineColorId = NoUniform;
    GLint m_ctmId = NoUniform;
    std::string m_suffix;
};

class LogarithmicAxisRenderer : public AxisRenderer
{
public:
    LogarithmicAxisRenderer(Range valuesRange,
                            const glm::vec2& axisDirection,
                            AxisDockPosition axisDockPosition,
                            ImFont* font,
                            int detailOrder = 0);

    float getWorldToAxis(float worldCoordinate) const override;

    float getAxisToWorld(float axisCoordinate) const override;

private:
    Range m_valuesRange;
};
}
