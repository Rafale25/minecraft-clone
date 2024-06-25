#pragma once

#include <glad/gl.h>

struct ChunkMesh {
    GLuint VAO, VBO, EBO;
    GLuint ssbo_texture_handles;
    size_t indices_count;
    bool is_initialized;

    ChunkMesh(): indices_count(0), is_initialized(false) {}

    void deleteAll()
    {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
        glDeleteBuffers(1, &ssbo_texture_handles);

        is_initialized = false;
    }
};
