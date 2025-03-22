#pragma once

#include "glm/exponential.hpp"
#include <glm/ext/vector_float3.hpp>

// Exponential decay constant
// useful range approx. 1 to 25, from slow to fast
inline glm::vec3 expDecay(const glm::vec3& a, const glm::vec3& b, float decay, float delta_time)
{
    return b + (a - b) * glm::exp(-decay * delta_time);
}

inline float expDecay(float a, float b, float decay, float delta_time)
{
    return b + (a - b) * glm::exp(-decay * delta_time);
}
