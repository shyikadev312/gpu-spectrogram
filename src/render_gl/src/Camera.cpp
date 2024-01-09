#include <spectr/render_gl/Camera.h>

namespace spectr::render_gl
{
Camera::Camera() = default;

const glm::mat3 Camera::getMvp() const
{
    glm::mat3 mvp{ glm::vec3{ m_scale.x, 0.0f, 0.0f },
                   glm::vec3{ 0.0f, m_scale.y, 0.0f },
                   glm::vec3{ -m_position.x, -m_position.y, 1.0f } };
    return mvp;
}

const glm::mat3 Camera::getMvpInv() const
{
    return glm::inverse(getMvp());
}

const glm::vec2& Camera::getPosition() const
{
    return m_position;
}

void Camera::setPosition(const glm::vec2& position)
{
    m_position = position;
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
}
