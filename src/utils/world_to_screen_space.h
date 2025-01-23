#pragma once

#include <glm/glm.hpp>

glm::ivec2 worldToScreenSpace(const glm::vec3& world_pos, const glm::mat4& projection, const glm::mat4& view, float screen_width, float screen_height)
{
    const glm::mat4 world_to_clip_matrix = projection * view;
    glm::vec4 clip_pos = world_to_clip_matrix * glm::vec4(world_pos, 1.0);
    clip_pos /= clip_pos.w;
    glm::vec2 screen_pos = clip_pos / 2.0f + 0.5f;
    screen_pos.x *= screen_width;
    screen_pos.y *= screen_height;
    screen_pos.y = screen_height - screen_pos.y;

    return screen_pos;
}
