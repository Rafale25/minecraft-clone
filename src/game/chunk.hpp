#pragma once

#include <glm/glm.hpp>
#include <glad/gl.h>

#include "enums.hpp"
#include "chunk_mesh.hpp"

class TextureManager;
class World;

struct Chunk
{
    glm::ivec3 pos;
    ChunkMesh mesh;

    BlockType blocks[4096]; // 16x16x16

    static int XYZtoIndex(int x, int y, int z);
    void computeChunckVAO(World& world, TextureManager& texture_manager);
};
