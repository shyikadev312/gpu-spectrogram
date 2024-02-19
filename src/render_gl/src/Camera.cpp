#include <spectr/render_gl/Camera.h>

namespace spectr::render_gl
{
const glm::mat3 Camera::getViewProjection() const
{
    glm::mat3 mvp{ glm::vec3{ m_scale.x, 0.0f, 0.0f },
                   glm::vec3{ 0.0f, m_scale.y, 0.0f },
                   glm::vec3{ -m_position.x, -m_position.y, 1.0f } };
    return mvp;
}

const glm::mat3 Camera::getViewProjectionInv() const
{
    return glm::inverse(getViewProjection());
}

const glm::vec2 Camera::getPositionWorld() const
{
    return m_position / m_scale;
}

void Camera::setPositionWorld(const glm::vec2& position)
{
    m_position = position * m_scale;
}

const glm::vec2& Camera::getScale() const
{
    return m_scale;
}

void Camera::setScale(const glm::vec2& scale)
{
    m_scale = scale;
}

float Camera::getScaleX() const
{
    return m_scale.x;
}

void Camera::setScaleX(float scaleX)
{
    m_scale.x = scaleX;
}

float Camera::getScaleY() const
{
    return m_scale.y;
}

void Camera::setScaleY(float scaleY)
{
    m_scale.y = scaleY;
}

glm::vec2 Camera::getViewportSize() const
{
    return 2.0f / getScale();
}
}
