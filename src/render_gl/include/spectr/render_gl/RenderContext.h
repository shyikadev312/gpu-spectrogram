#pragma once

#include <spectr/render_gl/Camera.h>

#include <memory>

namespace spectr::render_gl
{
class RenderContext
{
public:
    // to clip or to ndc?
    glm::vec3 ndcToWorld(const glm::vec2 ndcCoords) const;

    glm::vec2 worldToNdc(const glm::vec3 worldCoords) const;

    glm::vec2 pixelToNdc(const glm::ivec2& pixelCoords) const;

    glm::ivec2 ndcToPixel(const glm::vec2& ndcCoords) const;

    glm::vec2 pixelToWorld(const glm::ivec2& pixelCoords) const;

    glm::ivec2 worldToPixel(const glm::vec2& worldCoords) const;

    std::shared_ptr<Camera> camera;

    glm::ivec2 viewportSize;
};
}
