#pragma once
#include <glm/ext/vector_int3.hpp>
#include <glm/common.hpp>

inline bool isInManhattanDistance(const glm::ivec3& a, const glm::ivec3& b, int32_t distance)
{
    const glm::ivec3 v = glm::abs(a - b);
    return v.x <= distance && v.y <= distance && v.z <= distance;
}
