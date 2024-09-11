#pragma once

#include "glm/glm.hpp"
#include "enums.hpp"

struct BlockRaycastHit {
    BlockType blocktype;
    glm::ivec3 pos;
    glm::vec3 normal;
};
