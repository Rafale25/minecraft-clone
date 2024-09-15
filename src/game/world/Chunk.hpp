#pragma once

#include <glm/glm.hpp>
#include <glad/gl.h>

#include "enums.hpp"
#include "ChunkMesh.hpp"

class TextureManager;
class World;

struct Chunk
{
    // TODO: (maybe) disable new operator and have only the world able to allocate chunks

    glm::ivec3 pos;
    ChunkMesh mesh;

    // TODO: add is_monotype variable
    // bool is_monotype;
    BlockType blocks[4096]; // 16x16x16

    static int XYZtoIndex(int x, int y, int z);
};
