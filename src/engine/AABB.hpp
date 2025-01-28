#pragma once

#include <glm/detail/type_vec3.hpp>

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
};
