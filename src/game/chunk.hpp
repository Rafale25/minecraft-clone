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
    // GLuint VAO;
    // GLuint ssbo_texture_handles;
    // uint vertex_count;

    // Note: this is not a pointer, it gets copied
    BlockType blocks[4096]; // 16x16x16

    static int XYZtoIndex(int x, int y, int z);
    void computeChunckVAO(World& world, TextureManager& texture_manager);
};
