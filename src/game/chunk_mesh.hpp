#pragma once

#include <vector>
#include <glad/gl.h>

class World;
class TextureManager;
struct Chunk;

struct ChunkMesh {
    GLuint VAO, VBO, EBO;
    GLsizei indices_count;

    std::vector<GLuint> vertices;
    std::vector<GLuint> ebo;

    ChunkMesh(): VAO(0), VBO(0), EBO(0), indices_count(0) {}
    ~ChunkMesh() = default;

    void computeVertexBuffer(const World &world, const Chunk* chunk);
    void updateVAO();

    void deleteAll();
};
