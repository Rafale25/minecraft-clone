#pragma once

#include <vector>
#include <glad/gl.h>
#include <glm/glm.hpp>

#include "enums.hpp"
#include "BufferAllocator.hpp"

struct Chunk;
struct ChunkExtra;
class BufferAllocator;

struct ChunkRawMesh {
    std::vector<GLuint> vertices;
    std::vector<GLuint> indices;
};

struct ChunkMesh {
    BufferSlot slot_vertices = invalid_buffer_slot;
    BufferSlot slot_indices = invalid_buffer_slot;

    void updateVAO(
        BufferAllocator& buffer_allocator_vertices,
        BufferAllocator& buffer_allocator_indices,
        const BufferSlot& slot_vertices,
        const BufferSlot& slot_indices,
        const ChunkRawMesh& raw_mesh
    );
};

// ChunkRawMesh computeVertexBuffer(const Chunk *chunk);
ChunkRawMesh computeVertexBuffer(const glm::ivec3& chunk_pos);
