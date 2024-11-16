#include "ChunkMesh.hpp"

#include <stdio.h>

#include "World.hpp"
#include "BlockTextureManager.hpp"
#include "ChunkExtra.hpp"
#include "Chunk.hpp"


GLuint packVertex(int x, int y, int z, int u, int v, int o, int t, int ao=3) {
    // 4 bytes, 32 bits
    // 00000000000000000000000000000000
    //  aaattttttttooouvzzzzzyyyyyxxxxx
    GLuint p =
        ((x & 31)   << 0)   |
        ((y & 31)   << 5)   |
        ((z & 31)   << 10)  |
        ((u & 1)    << 15)  |
        ((v & 1)    << 16)  |
        ((o & 7)    << 17)  |
        ((t & 255)  << 20)  |
        ((ao & 7)   << 28);

    return p;
}

int vertexAO(int side1, int side2, int corner) {
    if (side1 && side2) return 0;
    return 3 - (side1 + side2 + corner);
}

const glm::ivec3 orientation_dir[] = {
    [Orientation::Top]     = glm::ivec3(0, 1, 0),
    [Orientation::Bottom]  = glm::ivec3(0, -1, 0),
    [Orientation::Front]   = glm::ivec3(0, 0, -1),
    [Orientation::Back]    = glm::ivec3(0, 0, 1),
    [Orientation::Left]    = glm::ivec3(-1, 0, 0),
    [Orientation::Right]   = glm::ivec3(1, 0, 0),
};

const int infos[][50] = {
    [Orientation::Top] = {
     // x, y, z,    u, v
        0, 1, 0,    0, 0,
        1, 1, 0,    1, 0,
        1, 1, 1,    1, 1,
        0, 1, 1,    0, 1,

     // indices
        0, 1, 2,
        0, 2, 3,

     // nb_lx
        -1, 1, 0,
     // nb_hx
        1, 1, 0,
     // nb_ly
        0, 1, -1,
     // nb_hy
        0, 1, 1,

     // nb_lxly
        -1, 1, -1,
     // nb_hxly
        1, 1, -1,
     // nb_lxhy
        -1, 1, 1,
     // nb_hxhy
        1, 1, 1,
    },

    [Orientation::Bottom] = {
        0, 0, 0,    0, 0,
        1, 0, 0,    1, 0,
        1, 0, 1,    1, 1,
        0, 0, 1,    0, 1,

        0, 2, 1,
        0, 3, 2,

        -1, -1, 0,
        1, -1, 0,
        0, -1, -1,
        0, -1, 1,

        -1, -1, -1,
        1, -1, -1,
        -1, -1, 1,
        1, -1, 1,
    },

    [Orientation::Front] = {
        0, 0, 0,  0, 0,
        1, 0, 0,  1, 0,
        1, 1, 0,  1, 1,
        0, 1, 0,  0, 1,

        0, 1, 2,
        0, 2, 3,

        -1, 0, -1,
        1, 0, -1,
        0, -1, -1,
        0, 1, -1,

        -1, -1, -1,
        1, -1, -1,
        -1, 1, -1,
        1, 1, -1,
    },

    [Orientation::Back] = {
        0, 0, 1,     0, 0,
        1, 0, 1,     1, 0,
        1, 1, 1,     1, 1,
        0, 1, 1,     0, 1,

        0, 2, 1,
        0, 3, 2,

        -1, 0, 1,
        1, 0, 1,
        0, -1, 1,
        0, 1, 1,

        -1, -1, 1,
        1, -1, 1,
        -1, 1, 1,
        1, 1, 1,
    },

    [Orientation::Left] = {
        0, 0, 0,    0, 0,
        0, 1, 0,    0, 1,
        0, 1, 1,    1, 1,
        0, 0, 1,    1, 0,

        0, 2, 3,
        0, 1, 2,

        -1, -1, 0,
        -1, 1, 0,
        -1, 0, -1,
        -1, 0, 1,

        -1, -1, -1,
        -1, 1, -1,
        -1, -1, 1,
        -1, 1, 1,
    },

    [Orientation::Right] = {
        1, 0, 0,    0, 0,
        1, 0, 1,    1, 0,
        1, 1, 1,    1, 1,
        1, 1, 0,    0, 1,

        0, 1, 2,
        0, 2, 3,

        1, 0, -1,
        1, 0, 1,
        1, -1, 0,
        1, 1, 0,

        1, -1, -1,
        1, -1, 1,
        1, 1, -1,
        1, 1, 1,
    },
};

inline void ChunkMesh::makeFace(
    int x, int y, int z,
    const ChunkExtra &chunkextra,
    GLuint& ebo_offset,
    const glm::ivec3& local_pos,
    Orientation orientation,
    GLuint texture_id
){
    glm::ivec3 dir = orientation_dir[orientation];

    const int* info = infos[orientation];

    // front
    BlockType nb = chunkextra.getBlock(local_pos + dir);
    BlockMetadata nbmtd = blocksMetadata[(int)nb];
    if (nbmtd.transparent) {

        // TODO: use blocksMetadata to check if it's non transparent instead of >0
        auto nb_lx = chunkextra.getBlock(local_pos + glm::ivec3(info[26], info[27], info[28])) > 0;
        auto nb_hx = chunkextra.getBlock(local_pos + glm::ivec3(info[29], info[30], info[31])) > 0;
        auto nb_ly = chunkextra.getBlock(local_pos + glm::ivec3(info[32], info[33], info[34])) > 0;
        auto nb_hy = chunkextra.getBlock(local_pos + glm::ivec3(info[35], info[36], info[37])) > 0;

        auto nb_lxly = chunkextra.getBlock(local_pos + glm::ivec3(info[38], info[39], info[40])) > 0;
        auto nb_hxly = chunkextra.getBlock(local_pos + glm::ivec3(info[41], info[42], info[43])) > 0;
        auto nb_lxhy = chunkextra.getBlock(local_pos + glm::ivec3(info[44], info[45], info[46])) > 0;
        auto nb_hxhy = chunkextra.getBlock(local_pos + glm::ivec3(info[47], info[48], info[49])) > 0;

        int a00 = vertexAO(nb_lx, nb_ly, nb_lxly);
        int a10 = vertexAO(nb_hx, nb_ly, nb_hxly);
        int a11 = vertexAO(nb_hx, nb_hy, nb_hxhy);
        int a01 = vertexAO(nb_lx, nb_hy, nb_lxhy);

        if(a00 + a11 > a01 + a10) {
            // generate normal quad
            vertices.insert(vertices.end(), {
                packVertex(x+info[0],  y+info[1],  z+info[2],  info[3],  info[4],  orientation, texture_id, a00),
                packVertex(x+info[5],  y+info[6],  z+info[7],  info[8],  info[9],  orientation, texture_id, a10),
                packVertex(x+info[10], y+info[11], z+info[12], info[13], info[14], orientation, texture_id, a11),
                packVertex(x+info[15], y+info[16], z+info[17], info[18], info[19], orientation, texture_id, a01),
            });

            ebo.insert(ebo.end(), {
                ebo_offset+info[20], ebo_offset+info[21], ebo_offset+info[22],
                ebo_offset+info[23], ebo_offset+info[24], ebo_offset+info[25]
            });
        } else {
            // generate flipped quad
            vertices.insert(vertices.end(), {
                packVertex(x+info[15], y+info[16], z+info[17], info[18], info[19], orientation, texture_id, a01),
                packVertex(x+info[10], y+info[11], z+info[12], info[13], info[14], orientation, texture_id, a11),
                packVertex(x+info[5],  y+info[6],  z+info[7],  info[8],  info[9],  orientation, texture_id, a10),
                packVertex(x+info[0],  y+info[1],  z+info[2],  info[3],  info[4],  orientation, texture_id, a00),
            });

            ebo.insert(ebo.end(), {
                ebo_offset+info[20], ebo_offset+info[22], ebo_offset+info[21],
                ebo_offset+info[23], ebo_offset+info[25], ebo_offset+info[24]
            });
        }
        ebo_offset += 4;
    }
}

void ChunkMesh::computeVertexBuffer(const Chunk* chunk)
{
    // TODO: check if chunk is only air, then remove it from world

    /*
        position: 3x5
        uv: 2x1
        orientation: 3
        texture_id: 8
        // 4 bytes, 32 bits
        // 00000000000000000000000000000000
        //     ttttttttooouvzzzzzyyyyyxxxxx
    */

    ChunkExtra chunkextra = ChunkExtra::get(chunk->pos);

    vertices.clear();
    ebo.clear();

    GLuint ebo_offset = 0;

    for (int z = 0 ; z < 16 ; ++z) {
    for (int y = 0 ; y < 16 ; ++y) {
    for (int x = 0 ; x < 16 ; ++x) {
        const int index = z * 16*16 + y * 16 + x;
        const BlockType block = chunk->blocks[index];

        if (block == BlockType::Air) continue;

        const glm::ivec3 local_pos = glm::ivec3(x, y, z);
        auto [texture_top_handle, texture_side_handle, texture_bot_handle] = BlockTextureManager::Get().block_textures_ids[block];

        BlockType nb; // neighbour block
        BlockMetadata nbmtd; // neighbour block metadata

        makeFace(x, y, z, chunkextra, ebo_offset, local_pos, Orientation::Front, texture_side_handle);
        makeFace(x, y, z, chunkextra, ebo_offset, local_pos, Orientation::Back, texture_side_handle);
        makeFace(x, y, z, chunkextra, ebo_offset, local_pos, Orientation::Bottom, texture_bot_handle);
        makeFace(x, y, z, chunkextra, ebo_offset, local_pos, Orientation::Top, texture_top_handle);
        makeFace(x, y, z, chunkextra, ebo_offset, local_pos, Orientation::Left, texture_side_handle);
        makeFace(x, y, z, chunkextra, ebo_offset, local_pos, Orientation::Right, texture_side_handle);
    }
    }
    }

    indices_count = ebo.size();
}

void ChunkMesh::updateVAO()
{
    if (VBO == 0) {
        glCreateBuffers(1, &VBO);
        glCreateBuffers(1, &EBO);
    }

    glNamedBufferData(VBO, vertices.size() * sizeof(GLuint), &vertices[0], GL_STATIC_DRAW);
    glNamedBufferData(EBO, ebo.size() * sizeof(GLuint), &ebo[0], GL_STATIC_DRAW);

    vertices.clear();
    ebo.clear();
    vertices.shrink_to_fit();
    ebo.shrink_to_fit();
}

void ChunkMesh::deleteAll()
{
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}
