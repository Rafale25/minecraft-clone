#pragma once

#include <glm/glm.hpp>
#include <glad/gl.h>

#include "enums.hpp"
#include "ChunkMesh.hpp"
#include "Chunk.hpp"

#include "World.hpp"
#include <string.h>
#include <cstring>

#include "clock.h"

struct ChunkExtra
{
    BlockType blocks[18*18*18];

    // work from -1 to 16 included
    static int XYZtoIndex(int x, int y, int z) {
        return (x+1) + (y+1)*18 + (z+1)*18*18;
    }

    BlockType getBlock(const glm::ivec3& pos) const {
        return blocks[XYZtoIndex(pos.x, pos.y, pos.z)];
    }

    static ChunkExtra get(const glm::ivec3& pos) {
        ChunkExtra chunkextra = {}; // need to be initialized to 0 (makes it like there's airblock if chunk aren't found)
        // ChunkExtra chunkextra_test = {};

        // memset(chunkextra.blocks,      0, 18*18*18);
        // memset(chunkextra_test.blocks, 0, 18*18*18);

        const std::shared_lock<std::shared_mutex> lock(World::instance().chunks_mutex);
        // const std::lock_guard<std::shared_mutex> lock(World::instance().chunks_mutex);

        // GOOD DATA //
        // for (int z = 0 ; z < 18 ; ++z) {
        // for (int y = 0 ; y < 18 ; ++y) {
        // for (int x = 0 ; x < 18 ; ++x) {
        //     int index = x + y*18 + z*18*18;
        //     glm::ivec3 world_pos = (pos * 16) + glm::ivec3(x-1, y-1, z-1);
        //     chunkextra_test.blocks[index] = World::instance().getBlock(world_pos);
        // }}}
        // ----- //

        // for (int z = 0 ; z < 18 ; ++z) {
        // for (int y = 0 ; y < 18 ; ++y) {
        // for (int x = 0 ; x < 18 ; ++x) {
        //    if (z != 0 && y != 0 && x != 0 && z != 17 && y != 17 && x != 17) continue; // middle chunk is already copied so skip it

        //     int index = x + y*18 + z*18*18;
        //     glm::ivec3 world_pos = (pos * 16) + glm::ivec3(x-1, y-1, z-1);
        //     chunkextra.blocks[index] = World::instance().getBlock(world_pos);
        // }}}

        /* Directly copy memory of chunks instead of using getBlock() */

        #define CORNERS_AND_DIAGONALS
        #ifdef CORNERS_AND_DIAGONALS
        // Corners //
        const Chunk* chunk_lxlylz = World::instance().getChunkUnsafe(pos + glm::ivec3(-1, -1, -1));
        if (chunk_lxlylz != nullptr) {
            int index = Chunk::XYZtoIndex(15, 15, 15);
            int index_chunk_extra = ChunkExtra::XYZtoIndex(-1, -1, -1);
            chunkextra.blocks[index_chunk_extra] = chunk_lxlylz->blocks[index];
        }

        const Chunk* chunk_hxlylz = World::instance().getChunkUnsafe(pos + glm::ivec3(1, -1, -1));
        if (chunk_hxlylz != nullptr) {
            int index = Chunk::XYZtoIndex(0, 15, 15);
            int index_chunk_extra = ChunkExtra::XYZtoIndex(16, -1, -1);
            chunkextra.blocks[index_chunk_extra] = chunk_hxlylz->blocks[index];
        }

        const Chunk* chunk_lxlyhz = World::instance().getChunkUnsafe(pos + glm::ivec3(-1, -1, 1));
        if (chunk_lxlyhz != nullptr) {
            int index = Chunk::XYZtoIndex(15, 15, 0);
            int index_chunk_extra = ChunkExtra::XYZtoIndex(-1, -1, 16);
            chunkextra.blocks[index_chunk_extra] = chunk_lxlyhz->blocks[index];
        }

        const Chunk* chunk_hxlyhz = World::instance().getChunkUnsafe(pos + glm::ivec3(1, -1, 1));
        if (chunk_hxlyhz != nullptr) {
            int index = Chunk::XYZtoIndex(0, 15, 0);
            int index_chunk_extra = ChunkExtra::XYZtoIndex(16, -1, 16);
            chunkextra.blocks[index_chunk_extra] = chunk_hxlyhz->blocks[index];
        }

        const Chunk* chunk_lxhylz = World::instance().getChunkUnsafe(pos + glm::ivec3(-1, 1, -1));
        if (chunk_lxhylz != nullptr) {
            int index = Chunk::XYZtoIndex(15, 0, 15);
            int index_chunk_extra = ChunkExtra::XYZtoIndex(-1, 16, -1);
            chunkextra.blocks[index_chunk_extra] = chunk_lxhylz->blocks[index];
        }

        const Chunk* chunk_hxhylz = World::instance().getChunkUnsafe(pos + glm::ivec3(1, 1, -1));
        if (chunk_hxhylz != nullptr) {
            int index = Chunk::XYZtoIndex(0, 0, 15);
            int index_chunk_extra = ChunkExtra::XYZtoIndex(16, 16, -1);
            chunkextra.blocks[index_chunk_extra] = chunk_hxhylz->blocks[index];
        }

        const Chunk* chunk_lxhyhz = World::instance().getChunkUnsafe(pos + glm::ivec3(-1, 1, 1));
        if (chunk_lxhyhz != nullptr) {
            int index = Chunk::XYZtoIndex(15, 0, 0);
            int index_chunk_extra = ChunkExtra::XYZtoIndex(-1, 16, 16);
            chunkextra.blocks[index_chunk_extra] = chunk_lxhyhz->blocks[index];
        }

        const Chunk* chunk_hxhyhz = World::instance().getChunkUnsafe(pos + glm::ivec3(1, 1, 1));
        if (chunk_hxhyhz != nullptr) {
            int index = Chunk::XYZtoIndex(0, 0, 0);
            int index_chunk_extra = ChunkExtra::XYZtoIndex(16, 16, 16);
            chunkextra.blocks[index_chunk_extra] = chunk_hxhyhz->blocks[index];
        }

        // Diagonals //
        const Chunk* chunk_lxly = World::instance().getChunkUnsafe(pos + glm::ivec3(-1, -1, 0));
        if (chunk_lxly != nullptr) {
            for (int i = 0 ; i < 16 ; ++i) {
                int index = Chunk::XYZtoIndex(15, 15, i);
                int index_chunk_extra = ChunkExtra::XYZtoIndex(-1, -1, i);
                chunkextra.blocks[index_chunk_extra] = chunk_lxly->blocks[index];
            }
        }

        const Chunk* chunk_hxly = World::instance().getChunkUnsafe(pos + glm::ivec3(1, -1, 0));
        if (chunk_hxly != nullptr) {
            for (int i = 0 ; i < 16 ; ++i) {
                int index = Chunk::XYZtoIndex(0, 15, i);
                int index_chunk_extra = ChunkExtra::XYZtoIndex(16, -1, i);
                chunkextra.blocks[index_chunk_extra] = chunk_hxly->blocks[index];
            }
        }

        const Chunk* chunk_lylz = World::instance().getChunkUnsafe(pos + glm::ivec3(0, -1, -1));
        if (chunk_lylz != nullptr) {
            for (int i = 0 ; i < 16 ; ++i) {
                int index = Chunk::XYZtoIndex(i, 15, 15);
                int index_chunk_extra = ChunkExtra::XYZtoIndex(i, -1, -1);
                chunkextra.blocks[index_chunk_extra] = chunk_lylz->blocks[index];
            }
        }

        const Chunk* chunk_lyhz = World::instance().getChunkUnsafe(pos + glm::ivec3(0, -1, 1));
        if (chunk_lyhz != nullptr) {
            for (int i = 0 ; i < 16 ; ++i) {
                int index = Chunk::XYZtoIndex(i, 15, 0);
                int index_chunk_extra = ChunkExtra::XYZtoIndex(i, -1, 16);
                chunkextra.blocks[index_chunk_extra] = chunk_lyhz->blocks[index];
            }
        }

        const Chunk* chunk_lxhy = World::instance().getChunkUnsafe(pos + glm::ivec3(-1, 1, 0));
        if (chunk_lxhy != nullptr) {
            for (int i = 0 ; i < 16 ; ++i) {
                int index = Chunk::XYZtoIndex(15, 0, i);
                int index_chunk_extra = ChunkExtra::XYZtoIndex(-1, 16, i);
                chunkextra.blocks[index_chunk_extra] = chunk_lxhy->blocks[index];
            }
        }

        const Chunk* chunk_hxhy = World::instance().getChunkUnsafe(pos + glm::ivec3(1, 1, 0));
        if (chunk_hxhy != nullptr) {
            for (int i = 0 ; i < 16 ; ++i) {
                int index = Chunk::XYZtoIndex(0, 0, i);
                int index_chunk_extra = ChunkExtra::XYZtoIndex(16, 16, i);
                chunkextra.blocks[index_chunk_extra] = chunk_hxhy->blocks[index];
            }
        }

        const Chunk* chunk_hylz = World::instance().getChunkUnsafe(pos + glm::ivec3(0, 1, -1));
        if (chunk_hylz != nullptr) {
            for (int i = 0 ; i < 16 ; ++i) {
                int index = Chunk::XYZtoIndex(i, 0, 15);
                int index_chunk_extra = ChunkExtra::XYZtoIndex(i, 16, -1);
                chunkextra.blocks[index_chunk_extra] = chunk_hylz->blocks[index];
            }
        }
        const Chunk* chunk_hyhz = World::instance().getChunkUnsafe(pos + glm::ivec3(0, 1, 1));
        if (chunk_hyhz != nullptr) {
            for (int i = 0 ; i < 16 ; ++i) {
                int index = Chunk::XYZtoIndex(i, 0, 0);
                int index_chunk_extra = ChunkExtra::XYZtoIndex(i, 16, 16);
                chunkextra.blocks[index_chunk_extra] = chunk_hyhz->blocks[index];
            }
        }

        const Chunk* chunk_lxlz = World::instance().getChunkUnsafe(pos + glm::ivec3(-1, 0, -1));
        if (chunk_lxlz != nullptr) {
            for (int i = 0 ; i < 16 ; ++i) {
                int index = Chunk::XYZtoIndex(15, i, 15);
                int index_chunk_extra = ChunkExtra::XYZtoIndex(-1, i, -1);
                chunkextra.blocks[index_chunk_extra] = chunk_lxlz->blocks[index];
            }
        }
        const Chunk* chunk_hxlz = World::instance().getChunkUnsafe(pos + glm::ivec3(1, 0, -1));
        if (chunk_hxlz != nullptr) {
            for (int i = 0 ; i < 16 ; ++i) {
                int index = Chunk::XYZtoIndex(0, i, 15);
                int index_chunk_extra = ChunkExtra::XYZtoIndex(16, i, -1);
                chunkextra.blocks[index_chunk_extra] = chunk_hxlz->blocks[index];
            }
        }
        const Chunk* chunk_lxhz = World::instance().getChunkUnsafe(pos + glm::ivec3(-1, 0, 1));
        if (chunk_lxhz != nullptr) {
            for (int i = 0 ; i < 16 ; ++i) {
                int index = Chunk::XYZtoIndex(15, i, 0);
                int index_chunk_extra = ChunkExtra::XYZtoIndex(-1, i, 16);
                chunkextra.blocks[index_chunk_extra] = chunk_lxhz->blocks[index];
            }
        }
        const Chunk* chunk_hxhz = World::instance().getChunkUnsafe(pos + glm::ivec3(1, 0, 1));
        if (chunk_hxhz != nullptr) {
            for (int i = 0 ; i < 16 ; ++i) {
                int index = Chunk::XYZtoIndex(0, i, 0);
                int index_chunk_extra = ChunkExtra::XYZtoIndex(16, i, 16);
                chunkextra.blocks[index_chunk_extra] = chunk_hxhz->blocks[index];
            }
        }
        #endif

        // Adjacents //
        const Chunk* middle_chunk = World::instance().getChunkUnsafe(pos);
        for (int z = 0 ; z < 16 ; ++z) {
        for (int y = 0 ; y < 16 ; ++y) {
        for (int x = 0 ; x < 16 ; ++x) {
            int index = Chunk::XYZtoIndex(x, y, z);
            int index_chunk_extra = ChunkExtra::XYZtoIndex(x, y, z);
            chunkextra.blocks[index_chunk_extra] = middle_chunk->blocks[index];
        }}}

        const Chunk* chunk_hz = World::instance().getChunkUnsafe(pos - glm::ivec3(0, 0, 1));
        if (chunk_hz != nullptr) {
            for (int y = 0 ; y < 16 ; ++y) {
            for (int x = 0 ; x < 16 ; ++x) {
                int index = Chunk::XYZtoIndex(x, y, 15);
                int index_chunk_extra = ChunkExtra::XYZtoIndex(x, y, -1);
                chunkextra.blocks[index_chunk_extra] = chunk_hz->blocks[index];
            }}
        }

        const Chunk* chunk_lz = World::instance().getChunkUnsafe(pos - glm::ivec3(0, 0, -1));
        if (chunk_lz != nullptr) {
            for (int y = 0 ; y < 16 ; ++y) {
            for (int x = 0 ; x < 16 ; ++x) {
                int index = Chunk::XYZtoIndex(x, y, 0);
                int index_chunk_extra = ChunkExtra::XYZtoIndex(x, y, 16);
                chunkextra.blocks[index_chunk_extra] = chunk_lz->blocks[index];
            }}
        }

        const Chunk* chunk_lx = World::instance().getChunkUnsafe(pos - glm::ivec3(-1, 0, 0));
        if (chunk_lx != nullptr) {
            for (int y = 0 ; y < 16 ; ++y) {
            for (int x = 0 ; x < 16 ; ++x) {
                int index = Chunk::XYZtoIndex(0, x, y);
                int index_chunk_extra = ChunkExtra::XYZtoIndex(16, x, y);
                chunkextra.blocks[index_chunk_extra] = chunk_lx->blocks[index];
            }}
        }

        const Chunk* chunk_hx = World::instance().getChunkUnsafe(pos - glm::ivec3(1, 0, 0));
        if (chunk_hx != nullptr) {
            for (int y = 0 ; y < 16 ; ++y) {
            for (int x = 0 ; x < 16 ; ++x) {
                int index = Chunk::XYZtoIndex(15, x, y);
                int index_chunk_extra = ChunkExtra::XYZtoIndex(-1, x, y);
                chunkextra.blocks[index_chunk_extra] = chunk_hx->blocks[index];
            }}
        }

        const Chunk* chunk_ly = World::instance().getChunkUnsafe(pos - glm::ivec3(0, -1, 0));
        if (chunk_ly != nullptr) {
            for (int y = 0 ; y < 16 ; ++y) {
            for (int x = 0 ; x < 16 ; ++x) {
                int index = Chunk::XYZtoIndex(x, 0, y);
                int index_chunk_extra = ChunkExtra::XYZtoIndex(x, 16, y);
                chunkextra.blocks[index_chunk_extra] = chunk_ly->blocks[index];
            }}
        }

        const Chunk* chunk_hy = World::instance().getChunkUnsafe(pos - glm::ivec3(0, 1, 0));
        if (chunk_hy != nullptr) {
            for (int y = 0 ; y < 16 ; ++y) {
            for (int x = 0 ; x < 16 ; ++x) {
                int index = Chunk::XYZtoIndex(x, 15, y);
                int index_chunk_extra = ChunkExtra::XYZtoIndex(x, -1, y);
                chunkextra.blocks[index_chunk_extra] = chunk_hy->blocks[index];
            }}
        }

        // int cmp = memcmp(chunkextra.blocks, chunkextra_test.blocks, 18*18*18);
        // if (cmp != 0) {
        //     printf("chunkextra is invalid %d\n", cmp);
        // }

        return chunkextra;
    }
};
