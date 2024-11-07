#pragma once

#include <glm/glm.hpp>
#include <glad/gl.h>

#include "enums.hpp"
#include "ChunkMesh.hpp"
#include "Chunk.hpp"

#include "World.hpp"
#include <string.h>
#include <cstring>

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
        ChunkExtra chunkextra_test;

        memset(chunkextra.blocks, 0, 18*18*18);
        memset(chunkextra_test.blocks, 0, 18*18*18);

        // const std::shared_lock<std::shared_mutex> lock(World::instance().chunks_mutex);
        const std::lock_guard<std::shared_mutex> lock(World::instance().chunks_mutex);

        // GOOD DATA //
        for (int z = 0 ; z < 18 ; ++z) {
        for (int y = 0 ; y < 18 ; ++y) {
        for (int x = 0 ; x < 18 ; ++x) {
            int index = x + y*18 + z*18*18;
            glm::ivec3 world_pos = (pos * 16) + glm::ivec3(x-1, y-1, z-1);
            chunkextra_test.blocks[index] = World::instance().getBlock(world_pos);
        }}}
        // ----- //

        // const Chunk* middle_chunk = World::instance().getChunk(pos);

        // Directly copy memory of center chunk instead of using getBlock
        // for (int z = 0 ; z < 16 ; ++z) {
        // for (int y = 0 ; y < 16 ; ++y) {
        // for (int x = 0 ; x < 16 ; ++x) {
        //     int index = Chunk::XYZtoIndex(x, y, z);
        //     int index_chunk_extra = ChunkExtra::XYZtoIndex(x, y, z);
        //     chunkextra.blocks[index_chunk_extra] = middle_chunk->blocks[index];
        // }}}

        for (int z = 0 ; z < 18 ; ++z) {
        for (int y = 0 ; y < 18 ; ++y) {
        for (int x = 0 ; x < 18 ; ++x) {
            // if (z != 0 && y != 0 && x != 0 && z != 17 && y != 17 && x != 17) continue; // middle chunk is already copied so skip it
            // if (x )

            int index = x + y*18 + z*18*18;
            glm::ivec3 world_pos = (pos * 16) + glm::ivec3(x-1, y-1, z-1);
            chunkextra.blocks[index] = World::instance().getBlock(world_pos);
        }}}

        // const Chunk* chunk_lx = World::instance().getChunk(pos - glm::ivec3(-1, 0, 0));
        // for (int x = 0 ; x < 16 ; ++x) {
        // for (int y = 0 ; y < 16 ; ++y) {
        //     int index = Chunk::XYZtoIndex(15, x, y);
        //     int index_chunk_extra = ChunkExtra::XYZtoIndex(0, y, x);
        // }}

        int cmp = memcmp(chunkextra.blocks, chunkextra_test.blocks, 18*18*18);
        if (cmp != 0) {
            printf("chunkextra is invalid %d\n", cmp);
        }

        return chunkextra;
    }
};
