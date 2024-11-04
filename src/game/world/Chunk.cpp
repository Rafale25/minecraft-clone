#include "Chunk.hpp"

int Chunk::XYZtoIndex(int x, int y, int z) {
    if (x < 0 || x > 15 || y < 0 || y > 15 || z < 0 || z > 15) return -1;
    return z * 16*16 + y * 16 + x;
}

uint Chunk::hash() {
    uint h = 1;

    for (int i = 0 ; i < 4096 ; ++i) {
        h *= (1779033703 + 2*(uint)blocks[i]);
    }

    return h;
}
