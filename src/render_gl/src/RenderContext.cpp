#include <spectr/render_gl/RenderContext.h>

namespace spectr::render_gl
{
glm::vec3 RenderContext::ndcToWorld(const glm::vec2 ndcCoords) const
{
    const auto worldPos = camera->getMvpInv() * glm::vec3(ndcCoords, 1.0f);
    return worldPos;
}

glm::vec2 RenderContext::worldToNdc(const glm::vec3 worldCoords) const
{
    const auto ndcPos = camera->getMvp() * worldCoords;
    // todo perspective division?
    return ndcPos;
}

glm::vec2 RenderContext::pixelToNdc(const glm::ivec2& pixelCoords) const
{
    const auto ratio = glm::vec2(pixelCoords) / glm::vec2(viewportSize);
    const auto ndcCoords = ratio * 2.0f - glm::vec2(1, 1);
    return ndcCoords;
}

glm::ivec2 RenderContext::ndcToPixel(const glm::vec2& ndcCoords) const
{
    const auto ratio = (ndcCoords + glm::vec2(1, 1)) / 2.0f;
    const auto ratioVerticalInverted = glm::vec2(ratio.x, ratio.y);
    const auto pixelCoords = ratio * glm::vec2(viewportSize);
    return pixelCoords;
}

glm::vec2 RenderContext::pixelToWorld(const glm::ivec2& pixelCoords) const
{
    const auto ndcCoords = pixelToNdc(pixelCoords);
    const auto worldCoords = camera->getMvpInv() * glm::vec3(ndcCoords, 1.0f);
    return glm::vec2(worldCoords.x, worldCoords.y);
}

glm::ivec2 RenderContext::worldToPixel(const glm::vec2& worldCoords) const
{
    const auto ndcCoords = camera->getMvp() * glm::vec3(worldCoords, 1.0f);
    const auto pixelCoords = ndcToPixel(ndcCoords);
    return pixelCoords;
}
}
