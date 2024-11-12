#pragma once

#include <vector>
#include <glad/gl.h>

#include <glm/glm.hpp>

#include "enums.hpp"

struct Chunk;
struct ChunkExtra;

struct ChunkMesh {
    GLuint VBO, EBO;
    GLsizei indices_count;

    std::vector<GLuint> vertices;
    std::vector<GLuint> ebo;

    ChunkMesh(): VBO(0), EBO(0), indices_count(0) {}
    ~ChunkMesh() = default;

    void makeFace(
        int x,
        int y,
        int z,
        const ChunkExtra &chunkextra,
        GLuint& ebo_offset,
        const glm::ivec3& local_pos,
        Orientation orientation,
        GLuint texture_id
    );

    void computeVertexBuffer(const Chunk *chunk);
    void updateVAO();

    void deleteAll();
};
