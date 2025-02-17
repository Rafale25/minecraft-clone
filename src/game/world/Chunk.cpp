#include "Chunk.hpp"

int32_t Chunk::XYZtoIndex(int32_t x, int32_t y, int32_t z) {
    if (x < 0 || x > 15 || y < 0 || y > 15 || z < 0 || z > 15) return -1;
    return z * 16*16 + y * 16 + x;
}

uint32_t Chunk::hash() {
    uint32_t h = 1;

    for (int32_t i = 0 ; i < 4096 ; ++i) {
        h *= (1779033703 + 2*(uint32_t)blocks[i]);
    }

    return h;
}
