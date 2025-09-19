#pragma once

#include "enums.hpp"
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_int3.hpp>

struct BlockRaycastHit {
    bool hit;
    BlockType blocktype;
    glm::ivec3 block_pos;
    glm::vec3 world_pos;
    glm::vec3 normal;
};
