#include "Chunk.hpp"
#include "constants.hpp"

int32_t Chunk::XYZtoIndex(int32_t x, int32_t y, int32_t z) {
    if (x < 0 || x > (CHUNK_SIZE-1) || y < 0 || y > (CHUNK_SIZE-1) || z < 0 || z > (CHUNK_SIZE-1)) return -1;
    return z * CHUNK_SIZE*CHUNK_SIZE + y * CHUNK_SIZE + x;
}

uint32_t Chunk::hash() {
    uint32_t h = 1;

    for (int32_t i = 0 ; i < CHUNK_BLOCK_COUNT ; ++i) {
        h *= (1779033703 + 2*(uint32_t)blocks[i]);
    }

    return h;
}
