#pragma once

#include "glm/glm.hpp"

// Exponential decay constant
// useful range approx. 1 to 25, from slow to fast
glm::vec3 expDecay(const glm::vec3& a, const glm::vec3& b, float decay, float delta_time)
{
    return b + (a - b) * expf(-decay * delta_time);
}

float expDecay(float a, float b, float decay, float delta_time)
{
    return b + (a - b) * expf(-decay * delta_time);
}
