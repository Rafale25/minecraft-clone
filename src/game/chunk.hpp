#pragma once

#include <glm/glm.hpp>
#include <glad/gl.h>

#include "enums.hpp"

class TextureManager;
class World;

struct Chunk
{
    glm::ivec3 pos;
    GLuint VAO;
    // bool vao_initialized = false;
    GLuint ssbo_texture_handles;
    uint vertices_count;

    // Note: this is not a pointer, it gets copied
    BlockType blocks[4096]; // 16x16x16

    static int XYZtoIndex(int x, int y, int z);
    void computeChunckVAO(World& world, TextureManager& texture_manager);
};
