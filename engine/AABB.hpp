#pragma once

#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_bool3.hpp>
#include <glm/common.hpp>

struct AABB {
	glm::vec3 min, max;

    static bool AABBtoAABB(const AABB& a, const AABB& b) {
        return (
            a.max.x > b.min.x &&
            a.min.x < b.max.x &&
            a.max.y > b.min.y &&
            a.min.y < b.max.y &&
            a.max.z > b.min.z &&
            a.min.z < b.max.z
        );
    }

    static glm::vec3 AABBtoAABBOverlapDistance(const AABB& a, const AABB& b) {
        const glm::vec3 min_minus_max = a.max - b.min;
        const glm::vec3 max_minus_min = a.min - b.max;
        const glm::bvec3 r = glm::lessThan(glm::abs(min_minus_max), glm::abs(max_minus_min));

        return glm::mix(max_minus_min, min_minus_max, r);
    }
};
