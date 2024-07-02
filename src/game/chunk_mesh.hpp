#pragma once

#include <glad/gl.h>

struct ChunkMesh {
    GLuint VAO, VBO, EBO;
    GLuint ssbo_texture_handles;
    GLsizei indices_count;

    ~ChunkMesh();

    ChunkMesh(): VAO(0), VBO(0), EBO(0), ssbo_texture_handles(0), indices_count(0) {}
    void deleteAll();
};
