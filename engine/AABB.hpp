#pragma once

#include <glm/ext/vector_float3.hpp>

struct AABB {
	glm::vec3 min, max;

    static bool AABBtoAABB(const AABB& a, const AABB& b);
    static glm::vec3 AABBtoAABBOverlapDistance(const AABB& a, const AABB& b);
};
