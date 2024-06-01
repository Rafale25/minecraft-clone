#pragma once

#include <glad/gl.h>

struct ChunkMesh {
    GLuint VAO;
    GLuint VBO;
    GLuint ssbo_texture_handles;
    uint32_t vertex_count;
    bool is_initialized;

    ChunkMesh(): is_initialized(false) {}

    void delete_all()
    {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &ssbo_texture_handles);

        is_initialized = false;
    }
};
