#pragma once

#include <glm/glm.hpp>

struct AABB {
	glm::vec3 min, max;

//     static bool AABBtoAABB(const AABB& a, const AABB& b) {
//         return (
//             a.high.x > b.low.x &&
//             a.low.x < b.high.x &&
//             a.high.y > b.low.y &&
//             a.low.y < b.high.y &&
//             a.high.z > b.low.z &&
//             a.low.z < b.high.z
//         );
//     }
};
