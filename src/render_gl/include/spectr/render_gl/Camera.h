#pragma once

#include <spectr/render_gl/GraphicsApi.h>

namespace spectr::render_gl
{
/**
 * @brief Camera for the 2D area.
 * @details Since camera is 2D only, the Z coordinate and rotation are not present.
 */
class Camera
{
public:
    Camera();

    const glm::mat3 getMvp() const;

    const glm::mat3 getMvpInv() const;

    const glm::vec2& getPosition() const;

    void setPosition(const glm::vec2& position);

    const glm::vec2& getScale() const;

    void setScale(const glm::vec2& scale);

    float getScaleX() const;

    void setScaleX(float scaleX);

    float getScaleY() const;

    void setScaleY(float scaleY);

private:
    glm::vec2 m_scale{ 1.0f, 1.0f };
    glm::vec2 m_position{ 0.0f, 0.0f };
};
}
