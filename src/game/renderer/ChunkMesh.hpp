#pragma once

#include <glm/ext/vector_int3.hpp>
#include "BufferAllocator.hpp"

struct ChunkRawMesh {
    std::vector<GLuint> vertices;

    // std::vector<GLuint> vertices_translucent;
};

struct ChunkMesh {
    BufferSlot slot_vertices = invalid_buffer_slot;

    // BufferSlot slot_vertices_translucent = invalid_buffer_slot;

    void updateVAO(
        BufferAllocator& buffer_allocator_vertices,
        const ChunkRawMesh& raw_mesh
    );
};

ChunkRawMesh computeVertexBuffer(const glm::ivec3& chunk_pos);
