#pragma once

#include "enums.hpp"
#include "constants.hpp"
#include <glm/ext/vector_int3.hpp>

struct Chunk
{
    // TODO: (maybe) disable new operator and have only the world able to allocate chunks
    glm::ivec3 pos;

    // TODO: add is_monotype variable
    // bool is_monotype;
    BlockType blocks[CHUNK_BLOCK_COUNT]; // 16x16x16

    static int32_t XYZtoIndex(int32_t x, int32_t y, int32_t z);

    uint32_t hash();
};
