#include <iostream>

#include "chunk.hpp"
#include "texture_manager.hpp"
#include "world.hpp"
#include "VAO.hpp"

int Chunk::XYZtoIndex(int x, int y, int z) {
    if (x < 0 || x > 15 || y < 0 || y > 15 || z < 0 || z > 15) return -1;
    return z * 16*16 + y * 16 + x;
}
