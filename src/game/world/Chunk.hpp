#pragma once

#include <glm/ext/vector_int3.hpp>
#include "enums.hpp"

struct Chunk
{
    // TODO: (maybe) disable new operator and have only the world able to allocate chunks
    glm::ivec3 pos;

    // TODO: add is_monotype variable
    // bool is_monotype;
    BlockType blocks[4096]; // 16x16x16

    static int XYZtoIndex(int x, int y, int z);

    uint32_t hash();
};
