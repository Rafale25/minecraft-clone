#pragma once

#include <vector>
#include <glad/gl.h>

struct Chunk;

struct ChunkMesh {
    GLuint VBO, EBO;
    GLsizei indices_count;

    std::vector<GLuint> vertices;
    std::vector<GLuint> ebo;

    ChunkMesh(): VBO(0), EBO(0), indices_count(0) {}
    ~ChunkMesh() = default;

    void computeVertexBuffer(const Chunk* chunk);
    void updateVAO();

    void deleteAll();
};
