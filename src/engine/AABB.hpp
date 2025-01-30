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

    static glm::vec3 AABBtoAABBOverlapDistance(const AABB& a, const AABB& b) {
        const glm::vec3 min_minus_max = a.max - b.min;
        const glm::vec3 max_minus_min = a.min - b.max;
        const glm::bvec3 r = glm::lessThan(glm::abs(min_minus_max), glm::abs(max_minus_min));

        return glm::mix(max_minus_min, min_minus_max, r);
    }

    // static glm::vec3 AABBtoAABBOverlapDistance(const AABB& a, const AABB& b) {
    //     glm::vec3 overlap;

    //     overlap.x = (glm::abs(a.max.x - b.min.x) < glm::abs(a.min.x - b.max.x)
    //                 ? a.max.x - b.min.x
    //                 : a.min.x - b.max.x);

    //     overlap.y = (glm::abs(a.max.y - b.min.y) < glm::abs(a.min.y - b.max.y)
    //                 ? a.max.y - b.min.y
    //                 : a.min.y - b.max.y);

    //     overlap.z = (glm::abs(a.max.z - b.min.z) < glm::abs(a.min.z - b.max.z)
    //                 ? a.max.z - b.min.z
    //                 : a.min.z - b.max.z);

    //     return overlap;
    // }
};
