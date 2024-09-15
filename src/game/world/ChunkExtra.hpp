#pragma once

#include <glm/glm.hpp>
#include <glad/gl.h>

#include "enums.hpp"
#include "ChunkMesh.hpp"
#include "Chunk.hpp"

#include "World.hpp"

struct ChunkExtra
{
    BlockType blocks[18*18*18];

    // work from -1 to 16 included
    static int XYZtoIndex(int x, int y, int z) {
        return (x+1) + (y+1)*18 + (z+1)*18*18;
    }

    BlockType getBlock(const glm::ivec3& pos) {
        return blocks[XYZtoIndex(pos.x, pos.y, pos.z)];
    }

    static ChunkExtra get(const glm::ivec3& pos) {
        ChunkExtra chunkextra;

        const std::shared_lock<std::shared_mutex> lock(World::instance().chunks_mutex);

        const Chunk* middle_chunk = World::instance().getChunk(pos);

        // Directly copy memory of center chunk instead of using getBlock
        for (int z = 0 ; z < 16 ; ++z) {
        for (int y = 0 ; y < 16 ; ++y) {
        for (int x = 0 ; x < 16 ; ++x) {
            int index = Chunk::XYZtoIndex(x, y, z);
            int index_chunk_extra = ChunkExtra::XYZtoIndex(x, y, z);
            chunkextra.blocks[index_chunk_extra] = middle_chunk->blocks[index];
        }
        }
        }

        for (int z = 0 ; z < 18 ; ++z) {
        for (int y = 0 ; y < 18 ; ++y) {
        for (int x = 0 ; x < 18 ; ++x) {
            if (z != 0 && y != 0 && x != 0 && z != 17 && y != 17 && x != 17) continue;

            int index = x + y*18 + z*18*18;
            glm::ivec3 world_pos = (pos * 16) + glm::ivec3(x-1, y-1, z-1);
            chunkextra.blocks[index] = World::instance().getBlock(world_pos);
        }
        }
        }

        return chunkextra;
    }
};
