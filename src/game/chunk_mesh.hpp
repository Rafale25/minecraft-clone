#pragma once

#include <glad/gl.h>

struct ChunkMesh {
    GLuint VAO, VBO, EBO;
    GLsizei indices_count;

    ChunkMesh(): VAO(0), VBO(0), EBO(0), indices_count(0) {}
    ~ChunkMesh();

    void deleteAll();
};
