#pragma once

#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_int2.hpp>
#include <glm/ext/matrix_float4x4.hpp>

inline glm::ivec2 worldToScreenSpace(const glm::vec3& world_pos, const glm::mat4& projection, const glm::mat4& view, float screenWidth, float screenHeight)
{
    const glm::mat4 worldToClipMatrix = projection * view;
    glm::vec4 clipPos = worldToClipMatrix * glm::vec4(world_pos, 1.0);
    clipPos /= clipPos.w;
    glm::vec2 screenPos = clipPos / 2.0f + 0.5f;
    screenPos.x *= screenWidth;
    screenPos.y *= screenHeight;
    screenPos.y = screenHeight - screenPos.y;

    return screenPos;
}
