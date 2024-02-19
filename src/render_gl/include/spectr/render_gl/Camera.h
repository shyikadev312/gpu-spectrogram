#pragma once

#include <spectr/render_gl/GraphicsApi.h>

namespace spectr::render_gl
{
/**
 * @brief Camera for the 2D area.
 * @details Since camera is 2D only, the Z coordinates are not present, projection matrix is
 * identity matrix, and the transformation matrices are 3x3.
 */
class Camera
{
public:
    /**
     * @brief Get view-projection matrix (world space to clip space transformation matrix).
     */
    const glm::mat3 getViewProjection() const;

    /**
     * @brief Get inverted view-projection matrix (clip space to world space transformation matrix).
     */
    const glm::mat3 getViewProjectionInv() const;

    /**
     * @brief Get camera position in world coordinates.
     */
    const glm::vec2 getPositionWorld() const;

    /**
     * @brief Set camera position in world coordinates.
     */
    void setPositionWorld(const glm::vec2& position);

    const glm::vec2& getScale() const;

    void setScale(const glm::vec2& scale);

    float getScaleX() const;

    void setScaleX(float scaleX);

    float getScaleY() const;

    void setScaleY(float scaleY);

    /**
     * @brief Get viewport size in world units.
     */
    glm::vec2 getViewportSize() const;

private:
    glm::vec2 m_scale{ 1.0f, 1.0f };
    glm::vec2 m_position{ 0.0f, 0.0f };
};
}
