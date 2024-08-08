#pragma once

#include <glm/glm.hpp>
#include <glad/gl.h>

#include "enums.hpp"
#include "chunk_mesh.hpp"

#include "world.hpp"

struct ChunkExtra
{
    BlockType blocks[18*18*18];

    // work from -1 to 16 included
    int XYZtoIndex(int x, int y, int z) {
        return (x+1) + (y+1)*18 + (z+1)*18*18;
    }

    BlockType getBlock(const glm::ivec3& pos) {
        return blocks[XYZtoIndex(pos.x, pos.y, pos.z)];
    }

    static ChunkExtra get(const World& world, const glm::ivec3& pos) {
        ChunkExtra chunkextra;

        const std::shared_lock lock(world.chunks_mutex);

        for (int z = 0 ; z < 18 ; ++z) {
        for (int y = 0 ; y < 18 ; ++y) {
        for (int x = 0 ; x < 18 ; ++x) {
            int index = x + y*18 + z*18*18;
            glm::ivec3 world_pos = (pos * 16) + glm::ivec3(x-1, y-1, z-1);
            chunkextra.blocks[index] = world.getBlock(world_pos);
        }
        }
        }

        return chunkextra;
    }
};
