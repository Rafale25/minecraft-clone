#pragma once

#include <glad/gl.h>
#include <vector>

struct ChunkMesh {
    GLuint VAO, VBO, EBO;
    GLsizei indices_count;

    std::vector<GLuint> vertices;
    std::vector<GLuint> ebo;

    ChunkMesh(): VAO(0), VBO(0), EBO(0), indices_count(0) {}
    ~ChunkMesh();

    void deleteAll();
};
