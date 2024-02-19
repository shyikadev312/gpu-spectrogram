#pragma once

#include <spectr/render_gl/RenderContext.h>

#include <glm/glm.hpp>

namespace spectr::desktop_app
{
class CameraBoundsController
{
public:
    CameraBoundsController(std::shared_ptr<render_gl::RenderContext> renderContext);

    void update();

    void setBounds(const glm::vec2& lowerLeft, const glm::vec2& upperRight);

    void setBoundsX(const glm::vec2& boundsX);
    
    void setBoundsY(const glm::vec2& boundsY);

    float getMinX() const;
    float getMaxX() const;
    float getMinY() const;
    float getMaxY() const;

private:
    glm::vec2 m_lowerLeft = { -100, -100 };
    glm::vec2 m_upperRight = { -100, -100 };
    std::shared_ptr<render_gl::RenderContext> m_renderContext;
};
}
