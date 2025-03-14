#include "ChunkMesh.hpp"

#include "World.hpp"
#include "BlockTextureManager.hpp"
#include "Chunk.hpp"
#include "ChunkExtra.hpp"
#include "BufferAllocator.hpp"

GLuint packVertex(int32_t x, int32_t y, int32_t z, int32_t u, int32_t v, int32_t o, int32_t t, int32_t ao=3) {
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

int32_t vertexAO(int32_t side1, int32_t side2, int32_t corner) {
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

const int32_t infos[][50] = {
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
    int32_t x, int32_t y, int32_t z,
    const ChunkExtra &chunkextra,
    GLuint& ebo_offset,
    const glm::ivec3& local_pos,
    Orientation orientation,
    GLuint texture_id
){
    glm::ivec3 dir = orientationToDir(orientation);

    const int32_t* info = infos[orientation];

    BlockType nb = chunkextra.getBlock(local_pos + dir);

    BlockInfo binfo = blocks_info[(int32_t)nb];
    if (binfo.transparent) {

        auto nb_lx = blocks_info[(int32_t)chunkextra.getBlock(local_pos + glm::ivec3(info[26], info[27], info[28]))].affectsAmbiantOcclusion;
        auto nb_hx = blocks_info[(int32_t)chunkextra.getBlock(local_pos + glm::ivec3(info[29], info[30], info[31]))].affectsAmbiantOcclusion;
        auto nb_ly = blocks_info[(int32_t)chunkextra.getBlock(local_pos + glm::ivec3(info[32], info[33], info[34]))].affectsAmbiantOcclusion;
        auto nb_hy = blocks_info[(int32_t)chunkextra.getBlock(local_pos + glm::ivec3(info[35], info[36], info[37]))].affectsAmbiantOcclusion;

        auto nb_lxly = blocks_info[(int32_t)chunkextra.getBlock(local_pos + glm::ivec3(info[38], info[39], info[40]))].affectsAmbiantOcclusion;
        auto nb_hxly = blocks_info[(int32_t)chunkextra.getBlock(local_pos + glm::ivec3(info[41], info[42], info[43]))].affectsAmbiantOcclusion;
        auto nb_lxhy = blocks_info[(int32_t)chunkextra.getBlock(local_pos + glm::ivec3(info[44], info[45], info[46]))].affectsAmbiantOcclusion;
        auto nb_hxhy = blocks_info[(int32_t)chunkextra.getBlock(local_pos + glm::ivec3(info[47], info[48], info[49]))].affectsAmbiantOcclusion;

        int32_t a00 = vertexAO(nb_lx, nb_ly, nb_lxly);
        int32_t a10 = vertexAO(nb_hx, nb_ly, nb_hxly);
        int32_t a11 = vertexAO(nb_hx, nb_hy, nb_hxhy);
        int32_t a01 = vertexAO(nb_lx, nb_hy, nb_lxhy);

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

    for (int32_t z = 0 ; z < CHUNK_SIZE ; ++z) {
    for (int32_t y = 0 ; y < CHUNK_SIZE ; ++y) {
    for (int32_t x = 0 ; x < CHUNK_SIZE ; ++x) {
        BlockType block = chunkextra.getBlock({x, y, z});

        if (block == BlockType::Air) continue;

        // If block has invalid BlockType, set it to invalid so it shows up with the INVALID texture
        if (block >= BlockType::INVALID) {
            block = BlockType::INVALID;
        }

        const glm::ivec3 local_pos = glm::ivec3(x, y, z);
        auto [texture_handle_lz, texture_handle_hz, texture_handle_lx, texture_handle_hx, texture_handle_ly, texture_handle_hy] = BlockTextureManager::Get().block_textures_ids[block];

        makeFace(chunk_raw_mesh.vertices, chunk_raw_mesh.indices, x, y, z, chunkextra, ebo_offset, local_pos, Orientation::Front, texture_handle_lz);
        makeFace(chunk_raw_mesh.vertices, chunk_raw_mesh.indices, x, y, z, chunkextra, ebo_offset, local_pos, Orientation::Back, texture_handle_hz);
        makeFace(chunk_raw_mesh.vertices, chunk_raw_mesh.indices, x, y, z, chunkextra, ebo_offset, local_pos, Orientation::Bottom, texture_handle_ly);
        makeFace(chunk_raw_mesh.vertices, chunk_raw_mesh.indices, x, y, z, chunkextra, ebo_offset, local_pos, Orientation::Top, texture_handle_hy);
        makeFace(chunk_raw_mesh.vertices, chunk_raw_mesh.indices, x, y, z, chunkextra, ebo_offset, local_pos, Orientation::Left, texture_handle_lx);
        makeFace(chunk_raw_mesh.vertices, chunk_raw_mesh.indices, x, y, z, chunkextra, ebo_offset, local_pos, Orientation::Right, texture_handle_hx);
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
