#include "ChunkMesh.hpp"

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

static inline glm::ivec3 orientationToDir(Orientation orientation) {
    switch (orientation) {
        case Orientation::Top:      return glm::ivec3(0, 1, 0);
        case Orientation::Bottom:   return glm::ivec3(0, -1, 0);
        case Orientation::Front:    return glm::ivec3(0, 0, -1);
        case Orientation::Back:     return glm::ivec3(0, 0, 1);
        case Orientation::Left:     return glm::ivec3(-1, 0, 0);
        case Orientation::Right:    return glm::ivec3(1, 0, 0);
        default:
            printf("Error: Invalid orientation");
            abort();
    }
}

// #pragma GCC diagnostic push
// #pragma GCC diagnostic ignored "-Wc99-designator"

const int infos[][50] = {
    // [Orientation::Top] 0
    {
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
    // [Orientation::Bottom] = 1
    {
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
    // [Orientation::Front] = 2
    {
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
    // [Orientation::Back] = 3
    {
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
    // [Orientation::Left] = 4
    {
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
    // [Orientation::Right] = 5
    {
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

// #pragma GCC diagnostic pop

inline void makeFace(
    std::vector<GLuint>& vertices,
    std::vector<GLuint>& indices,
    int x, int y, int z,
    const ChunkExtra &chunkextra,
    GLuint& ebo_offset,
    const glm::ivec3& local_pos,
    Orientation orientation,
    GLuint texture_id
){
    glm::ivec3 dir = orientationToDir(orientation);

    const int* info = infos[orientation];

    BlockType nb = chunkextra.getBlock(local_pos + dir);

    // if nb is invalid
    if (!(nb >= BlockType::Air && nb < BlockType::INVALID)) {
        nb = BlockType::Stone; // Assume not a transparent block so the face still get culled
    }
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

            indices.insert(indices.end(), {
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

            indices.insert(indices.end(), {
                ebo_offset+info[20], ebo_offset+info[22], ebo_offset+info[21],
                ebo_offset+info[23], ebo_offset+info[25], ebo_offset+info[24]
            });
        }
        ebo_offset += 4;
    }
}

// ChunkRawMesh computeVertexBuffer(const Chunk* chunk)
ChunkRawMesh computeVertexBuffer(const glm::ivec3& chunk_pos)
{
    /*
        position: 3x5
        uv: 2x1
        orientation: 3
        texture_id: 8
        // 4 bytes, 32 bits
        // 00000000000000000000000000000000
        //     ttttttttooouvzzzzzyyyyyxxxxx
    */

    ChunkExtra chunkextra = ChunkExtra::get(chunk_pos);
    ChunkRawMesh chunk_raw_mesh;

    GLuint ebo_offset = 0;

    for (int z = 0 ; z < 16 ; ++z) {
    for (int y = 0 ; y < 16 ; ++y) {
    for (int x = 0 ; x < 16 ; ++x) {
        BlockType block = chunkextra.getBlock({x, y, z});

        if (block == BlockType::Air) continue;

        // If block has invalid BlockType, set it to invalid so it shows up with the INVALID texture
        if (block >= BlockType::INVALID) {
            block = BlockType::INVALID;
        }

        const glm::ivec3 local_pos = glm::ivec3(x, y, z);
        auto [texture_top_handle, texture_side_handle, texture_bot_handle] = BlockTextureManager::Get().block_textures_ids[block];

        makeFace(chunk_raw_mesh.vertices, chunk_raw_mesh.indices, x, y, z, chunkextra, ebo_offset, local_pos, Orientation::Front, texture_side_handle);
        makeFace(chunk_raw_mesh.vertices, chunk_raw_mesh.indices, x, y, z, chunkextra, ebo_offset, local_pos, Orientation::Back, texture_side_handle);
        makeFace(chunk_raw_mesh.vertices, chunk_raw_mesh.indices, x, y, z, chunkextra, ebo_offset, local_pos, Orientation::Bottom, texture_bot_handle);
        makeFace(chunk_raw_mesh.vertices, chunk_raw_mesh.indices, x, y, z, chunkextra, ebo_offset, local_pos, Orientation::Top, texture_top_handle);
        makeFace(chunk_raw_mesh.vertices, chunk_raw_mesh.indices, x, y, z, chunkextra, ebo_offset, local_pos, Orientation::Left, texture_side_handle);
        makeFace(chunk_raw_mesh.vertices, chunk_raw_mesh.indices, x, y, z, chunkextra, ebo_offset, local_pos, Orientation::Right, texture_side_handle);
    }
    }
    }

    return chunk_raw_mesh;
}

void ChunkMesh::updateVAO(
    BufferAllocator& buffer_allocator_vertices,
    BufferAllocator& buffer_allocator_indices,
    const ChunkRawMesh& raw_mesh
){
    if (raw_mesh.vertices.size() == 0 || raw_mesh.indices.size() == 0) {
        return;
    }

    const int32_t vertices_size = raw_mesh.vertices.size() * sizeof(GLuint);
    const int32_t indices_size = raw_mesh.indices.size() * sizeof(GLuint);

    slot_vertices = buffer_allocator_vertices.allocate(vertices_size, &raw_mesh.vertices[0]);
    slot_indices = buffer_allocator_indices.allocate(indices_size, &raw_mesh.indices[0]);
}
