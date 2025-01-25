#pragma once

#include "glm/glm.hpp"
#include "enums.hpp"

struct BlockRaycastHit {
    BlockType blocktype;
    glm::ivec3 block_pos;
    glm::vec3 world_pos; // Not working as expected
    glm::vec3 normal;
};
