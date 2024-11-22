#pragma once

#include <vector>
#include <glad/gl.h>

#include <glm/glm.hpp>

#include "enums.hpp"

#include "BufferAllocator.hpp"

struct Chunk;
struct ChunkExtra;
class BufferAllocator;

struct ChunkMesh {
    BufferSlot slot_vertices = invalid_buffer_slot;
    BufferSlot slot_indices = invalid_buffer_slot;

    std::vector<GLuint> vertices;
    std::vector<GLuint> ebo;

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
    void updateVAO(BufferAllocator& buffer_allocator_vertices, BufferAllocator& buffer_allocator_indices, const BufferSlot& slot_vertices, const BufferSlot& slot_indices);

    void deleteAll();
};
