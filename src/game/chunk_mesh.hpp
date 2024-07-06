#pragma once

#include <glad/gl.h>

struct ChunkMesh {
    GLuint VAO, VBO, EBO;
    GLsizei indices_count;

    ~ChunkMesh();

    ChunkMesh(): VAO(0), VBO(0), EBO(0), indices_count(0) {}
    void deleteAll();
};
