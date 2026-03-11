#pragma once

#include "glm/exponential.hpp"
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_double3.hpp>

// Exponential decay constant
// useful range approx. 1 to 25, from slow to fast
inline glm::vec3 expDecay(const glm::vec3& a, const glm::vec3& b, float decay, float delta_time)
{
    return b + (a - b) * glm::exp(-decay * delta_time);
}

inline glm::vec3 expDecay(const glm::dvec3& a, const glm::dvec3& b, double decay, double delta_time)
{
    return b + (a - b) * (double)glm::exp(-decay * delta_time);
}

inline float expDecay(float a, float b, float decay, float delta_time)
{
    return b + (a - b) * glm::exp(-decay * delta_time);
}

inline double expDecay(double a, double b, double decay, double delta_time)
{
    return b + (a - b) * glm::exp(-decay * delta_time);
}
