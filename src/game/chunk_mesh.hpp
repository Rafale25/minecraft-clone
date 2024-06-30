#pragma once

#include <glad/gl.h>

struct ChunkMesh {
    GLuint VAO, VBO, EBO;
    GLuint ssbo_texture_handles;
    GLsizei indices_count;
    bool is_initialized;

    ChunkMesh(): indices_count(0), is_initialized(false) {}
    void deleteAll();
};
