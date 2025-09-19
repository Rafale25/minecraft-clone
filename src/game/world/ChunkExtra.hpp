#pragma once

#include "Chunk.hpp"
#include "World.hpp"
#include "enums.hpp"
#include "constants.hpp"

constexpr int32_t SIZE = CHUNK_SIZE + 2;

struct ChunkExtra
{
    BlockType blocks[SIZE*SIZE*SIZE];

    // work from -1 to CHUNK_SIZE included
    static int32_t XYZtoIndex(int32_t x, int32_t y, int32_t z) {
        return (x+1) + (y+1)*SIZE + (z+1)*SIZE*SIZE;
    }

    BlockType getBlock(const glm::ivec3& pos) const {
        const BlockType b = blocks[XYZtoIndex(pos.x, pos.y, pos.z)];
        if (b > BlockType::INVALID) return BlockType::INVALID;
        return b;
    }

    static ChunkExtra get(const glm::ivec3& pos) {
        ChunkExtra chunkextra = {}; // need to be initialized to 0 (makes it like there's airblock if chunk aren't found)
        // ChunkExtra chunkextra_test = {};

        // memset(chunkextra.blocks,      0, SIZE*SIZE*SIZE);
        // memset(chunkextra_test.blocks, 0, SIZE*SIZE*SIZE);

        const std::shared_lock<std::shared_mutex> lock(World::instance().chunks_mutex);

        // GOOD DATA //
        // for (int32_t z = 0 ; z < SIZE ; ++z) {
        // for (int32_t y = 0 ; y < SIZE ; ++y) {
        // for (int32_t x = 0 ; x < SIZE ; ++x) {
        //     int32_t index = x + y*SIZE + z*SIZE*SIZE;
        //     glm::ivec3 world_pos = (pos * CHUNK_SIZE) + glm::ivec3(x-1, y-1, z-1);
        //     chunkextra_test.blocks[index] = World::instance().getBlock(world_pos);
        // }}}
        // ----- //

        /* Directly copy memory of chunks instead of using getBlock() */
        // for (int32_t z = 0 ; z < SIZE ; ++z) {
        // for (int32_t y = 0 ; y < SIZE ; ++y) {
        // for (int32_t x = 0 ; x < SIZE ; ++x) {
        //    if (z != 0 && y != 0 && x != 0 && z != 17 && y != 17 && x != 17) continue; // middle chunk is already copied so skip it

        //     int32_t index = x + y*SIZE + z*SIZE*SIZE;
        //     glm::ivec3 world_pos = (pos * CHUNK_SIZE) + glm::ivec3(x-1, y-1, z-1);
        //     chunkextra.blocks[index] = World::instance().getBlock(world_pos);
        // }}}

        #define CORNERS_AND_DIAGONALS
        #ifdef CORNERS_AND_DIAGONALS
        // Corners //
        const Chunk* chunk_lxlylz = World::instance().getChunkUnsafe(pos + glm::ivec3(-1, -1, -1));
        if (chunk_lxlylz != nullptr) {
            int32_t index = Chunk::XYZtoIndex(CHUNK_SIZE-1, CHUNK_SIZE-1, CHUNK_SIZE-1);
            int32_t index_chunk_extra = ChunkExtra::XYZtoIndex(-1, -1, -1);
            chunkextra.blocks[index_chunk_extra] = chunk_lxlylz->blocks[index];
        }

        const Chunk* chunk_hxlylz = World::instance().getChunkUnsafe(pos + glm::ivec3(1, -1, -1));
        if (chunk_hxlylz != nullptr) {
            int32_t index = Chunk::XYZtoIndex(0, CHUNK_SIZE-1, CHUNK_SIZE-1);
            int32_t index_chunk_extra = ChunkExtra::XYZtoIndex(CHUNK_SIZE, -1, -1);
            chunkextra.blocks[index_chunk_extra] = chunk_hxlylz->blocks[index];
        }

        const Chunk* chunk_lxlyhz = World::instance().getChunkUnsafe(pos + glm::ivec3(-1, -1, 1));
        if (chunk_lxlyhz != nullptr) {
            int32_t index = Chunk::XYZtoIndex(CHUNK_SIZE-1, CHUNK_SIZE-1, 0);
            int32_t index_chunk_extra = ChunkExtra::XYZtoIndex(-1, -1, CHUNK_SIZE);
            chunkextra.blocks[index_chunk_extra] = chunk_lxlyhz->blocks[index];
        }

        const Chunk* chunk_hxlyhz = World::instance().getChunkUnsafe(pos + glm::ivec3(1, -1, 1));
        if (chunk_hxlyhz != nullptr) {
            int32_t index = Chunk::XYZtoIndex(0, CHUNK_SIZE-1, 0);
            int32_t index_chunk_extra = ChunkExtra::XYZtoIndex(CHUNK_SIZE, -1, CHUNK_SIZE);
            chunkextra.blocks[index_chunk_extra] = chunk_hxlyhz->blocks[index];
        }

        const Chunk* chunk_lxhylz = World::instance().getChunkUnsafe(pos + glm::ivec3(-1, 1, -1));
        if (chunk_lxhylz != nullptr) {
            int32_t index = Chunk::XYZtoIndex(CHUNK_SIZE-1, 0, CHUNK_SIZE-1);
            int32_t index_chunk_extra = ChunkExtra::XYZtoIndex(-1, CHUNK_SIZE, -1);
            chunkextra.blocks[index_chunk_extra] = chunk_lxhylz->blocks[index];
        }

        const Chunk* chunk_hxhylz = World::instance().getChunkUnsafe(pos + glm::ivec3(1, 1, -1));
        if (chunk_hxhylz != nullptr) {
            int32_t index = Chunk::XYZtoIndex(0, 0, CHUNK_SIZE-1);
            int32_t index_chunk_extra = ChunkExtra::XYZtoIndex(CHUNK_SIZE, CHUNK_SIZE, -1);
            chunkextra.blocks[index_chunk_extra] = chunk_hxhylz->blocks[index];
        }

        const Chunk* chunk_lxhyhz = World::instance().getChunkUnsafe(pos + glm::ivec3(-1, 1, 1));
        if (chunk_lxhyhz != nullptr) {
            int32_t index = Chunk::XYZtoIndex(CHUNK_SIZE-1, 0, 0);
            int32_t index_chunk_extra = ChunkExtra::XYZtoIndex(-1, CHUNK_SIZE, CHUNK_SIZE);
            chunkextra.blocks[index_chunk_extra] = chunk_lxhyhz->blocks[index];
        }

        const Chunk* chunk_hxhyhz = World::instance().getChunkUnsafe(pos + glm::ivec3(1, 1, 1));
        if (chunk_hxhyhz != nullptr) {
            int32_t index = Chunk::XYZtoIndex(0, 0, 0);
            int32_t index_chunk_extra = ChunkExtra::XYZtoIndex(CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE);
            chunkextra.blocks[index_chunk_extra] = chunk_hxhyhz->blocks[index];
        }

        // Diagonals //
        const Chunk* chunk_lxly = World::instance().getChunkUnsafe(pos + glm::ivec3(-1, -1, 0));
        if (chunk_lxly != nullptr) {
            for (int32_t i = 0 ; i < CHUNK_SIZE ; ++i) {
                int32_t index = Chunk::XYZtoIndex(CHUNK_SIZE-1, CHUNK_SIZE-1, i);
                int32_t index_chunk_extra = ChunkExtra::XYZtoIndex(-1, -1, i);
                chunkextra.blocks[index_chunk_extra] = chunk_lxly->blocks[index];
            }
        }

        const Chunk* chunk_hxly = World::instance().getChunkUnsafe(pos + glm::ivec3(1, -1, 0));
        if (chunk_hxly != nullptr) {
            for (int32_t i = 0 ; i < CHUNK_SIZE ; ++i) {
                int32_t index = Chunk::XYZtoIndex(0, CHUNK_SIZE-1, i);
                int32_t index_chunk_extra = ChunkExtra::XYZtoIndex(CHUNK_SIZE, -1, i);
                chunkextra.blocks[index_chunk_extra] = chunk_hxly->blocks[index];
            }
        }

        const Chunk* chunk_lylz = World::instance().getChunkUnsafe(pos + glm::ivec3(0, -1, -1));
        if (chunk_lylz != nullptr) {
            for (int32_t i = 0 ; i < CHUNK_SIZE ; ++i) {
                int32_t index = Chunk::XYZtoIndex(i, CHUNK_SIZE-1, CHUNK_SIZE-1);
                int32_t index_chunk_extra = ChunkExtra::XYZtoIndex(i, -1, -1);
                chunkextra.blocks[index_chunk_extra] = chunk_lylz->blocks[index];
            }
        }

        const Chunk* chunk_lyhz = World::instance().getChunkUnsafe(pos + glm::ivec3(0, -1, 1));
        if (chunk_lyhz != nullptr) {
            for (int32_t i = 0 ; i < CHUNK_SIZE ; ++i) {
                int32_t index = Chunk::XYZtoIndex(i, CHUNK_SIZE-1, 0);
                int32_t index_chunk_extra = ChunkExtra::XYZtoIndex(i, -1, CHUNK_SIZE);
                chunkextra.blocks[index_chunk_extra] = chunk_lyhz->blocks[index];
            }
        }

        const Chunk* chunk_lxhy = World::instance().getChunkUnsafe(pos + glm::ivec3(-1, 1, 0));
        if (chunk_lxhy != nullptr) {
            for (int32_t i = 0 ; i < CHUNK_SIZE ; ++i) {
                int32_t index = Chunk::XYZtoIndex(CHUNK_SIZE-1, 0, i);
                int32_t index_chunk_extra = ChunkExtra::XYZtoIndex(-1, CHUNK_SIZE, i);
                chunkextra.blocks[index_chunk_extra] = chunk_lxhy->blocks[index];
            }
        }

        const Chunk* chunk_hxhy = World::instance().getChunkUnsafe(pos + glm::ivec3(1, 1, 0));
        if (chunk_hxhy != nullptr) {
            for (int32_t i = 0 ; i < CHUNK_SIZE ; ++i) {
                int32_t index = Chunk::XYZtoIndex(0, 0, i);
                int32_t index_chunk_extra = ChunkExtra::XYZtoIndex(CHUNK_SIZE, CHUNK_SIZE, i);
                chunkextra.blocks[index_chunk_extra] = chunk_hxhy->blocks[index];
            }
        }

        const Chunk* chunk_hylz = World::instance().getChunkUnsafe(pos + glm::ivec3(0, 1, -1));
        if (chunk_hylz != nullptr) {
            for (int32_t i = 0 ; i < CHUNK_SIZE ; ++i) {
                int32_t index = Chunk::XYZtoIndex(i, 0, CHUNK_SIZE-1);
                int32_t index_chunk_extra = ChunkExtra::XYZtoIndex(i, CHUNK_SIZE, -1);
                chunkextra.blocks[index_chunk_extra] = chunk_hylz->blocks[index];
            }
        }
        const Chunk* chunk_hyhz = World::instance().getChunkUnsafe(pos + glm::ivec3(0, 1, 1));
        if (chunk_hyhz != nullptr) {
            for (int32_t i = 0 ; i < CHUNK_SIZE ; ++i) {
                int32_t index = Chunk::XYZtoIndex(i, 0, 0);
                int32_t index_chunk_extra = ChunkExtra::XYZtoIndex(i, CHUNK_SIZE, CHUNK_SIZE);
                chunkextra.blocks[index_chunk_extra] = chunk_hyhz->blocks[index];
            }
        }

        const Chunk* chunk_lxlz = World::instance().getChunkUnsafe(pos + glm::ivec3(-1, 0, -1));
        if (chunk_lxlz != nullptr) {
            for (int32_t i = 0 ; i < CHUNK_SIZE ; ++i) {
                int32_t index = Chunk::XYZtoIndex(CHUNK_SIZE-1, i, CHUNK_SIZE-1);
                int32_t index_chunk_extra = ChunkExtra::XYZtoIndex(-1, i, -1);
                chunkextra.blocks[index_chunk_extra] = chunk_lxlz->blocks[index];
            }
        }
        const Chunk* chunk_hxlz = World::instance().getChunkUnsafe(pos + glm::ivec3(1, 0, -1));
        if (chunk_hxlz != nullptr) {
            for (int32_t i = 0 ; i < CHUNK_SIZE ; ++i) {
                int32_t index = Chunk::XYZtoIndex(0, i, CHUNK_SIZE-1);
                int32_t index_chunk_extra = ChunkExtra::XYZtoIndex(CHUNK_SIZE, i, -1);
                chunkextra.blocks[index_chunk_extra] = chunk_hxlz->blocks[index];
            }
        }
        const Chunk* chunk_lxhz = World::instance().getChunkUnsafe(pos + glm::ivec3(-1, 0, 1));
        if (chunk_lxhz != nullptr) {
            for (int32_t i = 0 ; i < CHUNK_SIZE ; ++i) {
                int32_t index = Chunk::XYZtoIndex(CHUNK_SIZE-1, i, 0);
                int32_t index_chunk_extra = ChunkExtra::XYZtoIndex(-1, i, CHUNK_SIZE);
                chunkextra.blocks[index_chunk_extra] = chunk_lxhz->blocks[index];
            }
        }
        const Chunk* chunk_hxhz = World::instance().getChunkUnsafe(pos + glm::ivec3(1, 0, 1));
        if (chunk_hxhz != nullptr) {
            for (int32_t i = 0 ; i < CHUNK_SIZE ; ++i) {
                int32_t index = Chunk::XYZtoIndex(0, i, 0);
                int32_t index_chunk_extra = ChunkExtra::XYZtoIndex(CHUNK_SIZE, i, CHUNK_SIZE);
                chunkextra.blocks[index_chunk_extra] = chunk_hxhz->blocks[index];
            }
        }
        #endif

        // Adjacents //
        const Chunk* middle_chunk = World::instance().getChunkUnsafe(pos);
        if (middle_chunk != nullptr) {
            for (int32_t z = 0 ; z < CHUNK_SIZE ; ++z) {
            for (int32_t y = 0 ; y < CHUNK_SIZE ; ++y) {
            for (int32_t x = 0 ; x < CHUNK_SIZE ; ++x) {
                int32_t index = Chunk::XYZtoIndex(x, y, z);
                int32_t index_chunk_extra = ChunkExtra::XYZtoIndex(x, y, z);
                chunkextra.blocks[index_chunk_extra] = middle_chunk->blocks[index];
            }}}
        }

        const Chunk* chunk_hz = World::instance().getChunkUnsafe(pos - glm::ivec3(0, 0, 1));
        if (chunk_hz != nullptr) {
            for (int32_t y = 0 ; y < CHUNK_SIZE ; ++y) {
            for (int32_t x = 0 ; x < CHUNK_SIZE ; ++x) {
                int32_t index = Chunk::XYZtoIndex(x, y, CHUNK_SIZE-1);
                int32_t index_chunk_extra = ChunkExtra::XYZtoIndex(x, y, -1);
                chunkextra.blocks[index_chunk_extra] = chunk_hz->blocks[index];
            }}
        }

        const Chunk* chunk_lz = World::instance().getChunkUnsafe(pos - glm::ivec3(0, 0, -1));
        if (chunk_lz != nullptr) {
            for (int32_t y = 0 ; y < CHUNK_SIZE ; ++y) {
            for (int32_t x = 0 ; x < CHUNK_SIZE ; ++x) {
                int32_t index = Chunk::XYZtoIndex(x, y, 0);
                int32_t index_chunk_extra = ChunkExtra::XYZtoIndex(x, y, CHUNK_SIZE);
                chunkextra.blocks[index_chunk_extra] = chunk_lz->blocks[index];
            }}
        }

        const Chunk* chunk_lx = World::instance().getChunkUnsafe(pos - glm::ivec3(-1, 0, 0));
        if (chunk_lx != nullptr) {
            for (int32_t y = 0 ; y < CHUNK_SIZE ; ++y) {
            for (int32_t x = 0 ; x < CHUNK_SIZE ; ++x) {
                int32_t index = Chunk::XYZtoIndex(0, x, y);
                int32_t index_chunk_extra = ChunkExtra::XYZtoIndex(CHUNK_SIZE, x, y);
                chunkextra.blocks[index_chunk_extra] = chunk_lx->blocks[index];
            }}
        }

        const Chunk* chunk_hx = World::instance().getChunkUnsafe(pos - glm::ivec3(1, 0, 0));
        if (chunk_hx != nullptr) {
            for (int32_t y = 0 ; y < CHUNK_SIZE ; ++y) {
            for (int32_t x = 0 ; x < CHUNK_SIZE ; ++x) {
                int32_t index = Chunk::XYZtoIndex(CHUNK_SIZE-1, x, y);
                int32_t index_chunk_extra = ChunkExtra::XYZtoIndex(-1, x, y);
                chunkextra.blocks[index_chunk_extra] = chunk_hx->blocks[index];
            }}
        }

        const Chunk* chunk_ly = World::instance().getChunkUnsafe(pos - glm::ivec3(0, -1, 0));
        if (chunk_ly != nullptr) {
            for (int32_t y = 0 ; y < CHUNK_SIZE ; ++y) {
            for (int32_t x = 0 ; x < CHUNK_SIZE ; ++x) {
                int32_t index = Chunk::XYZtoIndex(x, 0, y);
                int32_t index_chunk_extra = ChunkExtra::XYZtoIndex(x, CHUNK_SIZE, y);
                chunkextra.blocks[index_chunk_extra] = chunk_ly->blocks[index];
            }}
        }

        const Chunk* chunk_hy = World::instance().getChunkUnsafe(pos - glm::ivec3(0, 1, 0));
        if (chunk_hy != nullptr) {
            for (int32_t y = 0 ; y < CHUNK_SIZE ; ++y) {
            for (int32_t x = 0 ; x < CHUNK_SIZE ; ++x) {
                int32_t index = Chunk::XYZtoIndex(x, CHUNK_SIZE-1, y);
                int32_t index_chunk_extra = ChunkExtra::XYZtoIndex(x, -1, y);
                chunkextra.blocks[index_chunk_extra] = chunk_hy->blocks[index];
            }}
        }

        // int32_t cmp = memcmp(chunkextra.blocks, chunkextra_test.blocks, SIZE*SIZE*SIZE);
        // if (cmp != 0) {
        //     printf("chunkextra is invalid %d\n", cmp);
        // }

        return chunkextra;
    }
};
