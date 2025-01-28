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
        glm::vec3 overlap;

        // Compute overlap distances for each axis
        overlap.x = (glm::abs(a.max.x - b.min.x) < glm::abs(a.min.x - b.max.x)
                    ? a.max.x - b.min.x
                    : a.min.x - b.max.x);

        overlap.y = (glm::abs(a.max.y - b.min.y) < glm::abs(a.min.y - b.max.y)
                    ? a.max.y - b.min.y
                    : a.min.y - b.max.y);

        overlap.z = (glm::abs(a.max.z - b.min.z) < glm::abs(a.min.z - b.max.z)
                    ? a.max.z - b.min.z
                    : a.min.z - b.max.z);

        return overlap;
    }


    // static glm::vec3 AABBtoAABBOverlapDistance(const AABB& a, const AABB& b) {
    //     glm::vec3 overlap;

    //     // Calculate overlap distance in the x-direction
    //     if (glm::abs(a.max.x - b.min.x) < glm::abs(a.min.x - b.max.x)) {
    //         overlap.x = a.max.x - b.min.x;
    //     } else {
    //         overlap.x = a.min.x - b.max.x;
    //     }

    //     // Calculate overlap distance in the y-direction
    //     if (glm::abs(a.max.y - b.min.y) < glm::abs(a.min.y - b.max.y)) {
    //         overlap.y = a.max.y - b.min.y;
    //     } else {
    //         overlap.y = a.min.y - b.max.y;
    //     }

    //     // Calculate overlap distance in the z-direction
    //     if (glm::abs(a.max.z - b.min.z) < glm::abs(a.min.z - b.max.z)) {
    //         overlap.z = a.max.z - b.min.z;
    //     } else {
    //         overlap.z = a.min.z - b.max.z;
    //     }

    //     return overlap;
    // }
};
