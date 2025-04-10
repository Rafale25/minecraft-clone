#include "ChunkMesh.hpp"
#include "BlockTextureManager.hpp"
#include "ChunkExtra.hpp"
#include "Logger.hpp"

GLuint64 packVertex(int32_t x, int32_t y, int32_t z, int32_t o, int32_t t, int32_t ao00, int32_t ao10, int32_t ao11, int32_t ao01) {
    // 8 bytes, 64 bits

    // ------------------------aaaaaaaa
    // --tttttttttooozzzzzzyyyyyyxxxxxx
    GLuint64 p = (GLuint64)
        ((GLuint64)(x & 63)   << 0)   |
        ((GLuint64)(y & 63)   << 6)   |
        ((GLuint64)(z & 63)   << 12)  |
        ((GLuint64)(o & 7)    << 18)  |
        ((GLuint64)(t & 511)  << 21) |

        (((GLuint64)(ao00 & 3)) << 32) |
        (((GLuint64)(ao10 & 3)) << 34) |
        (((GLuint64)(ao11 & 3)) << 36) |
        (((GLuint64)(ao01 & 3)) << 38);

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
            logE("Invalid orientation");
            abort();
    }
}

constexpr int32_t infos[][26] = {
    // [Orientation::Top] 0
    {
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

inline void makeFace(
    std::vector<GLuint64>& vertices,
    int32_t x, int32_t y, int32_t z,
    const ChunkExtra &chunkextra,
    const glm::ivec3& local_pos,
    Orientation orientation,
    GLuint texture_id
){
    glm::ivec3 dir = orientationToDir(orientation);

    const int32_t* info = infos[orientation];

    BlockType self_block = chunkextra.getBlock(local_pos);
    BlockType neighbor_block = chunkextra.getBlock(local_pos + dir);

    BlockInfo self_block_info = blocks_info[(int32_t)self_block];
    BlockInfo neighbor_block_info = blocks_info[(int32_t)neighbor_block];

    // Only mesh if neighbor is transparent or a if is a solid block next to a liquid
    if (!neighbor_block_info.transparent && !(!self_block_info.liquid && neighbor_block_info.liquid)) return;

    auto nb_lx = blocks_info[(int32_t)chunkextra.getBlock(local_pos + glm::ivec3(info[0], info[1], info[2]))].affectsAmbiantOcclusion;
    auto nb_hx = blocks_info[(int32_t)chunkextra.getBlock(local_pos + glm::ivec3(info[3], info[4], info[5]))].affectsAmbiantOcclusion;
    auto nb_ly = blocks_info[(int32_t)chunkextra.getBlock(local_pos + glm::ivec3(info[6], info[7], info[8]))].affectsAmbiantOcclusion;
    auto nb_hy = blocks_info[(int32_t)chunkextra.getBlock(local_pos + glm::ivec3(info[9], info[10], info[11]))].affectsAmbiantOcclusion;

    auto nb_lxly = blocks_info[(int32_t)chunkextra.getBlock(local_pos + glm::ivec3(info[12], info[13], info[14]))].affectsAmbiantOcclusion;
    auto nb_hxly = blocks_info[(int32_t)chunkextra.getBlock(local_pos + glm::ivec3(info[15], info[16], info[17]))].affectsAmbiantOcclusion;
    auto nb_lxhy = blocks_info[(int32_t)chunkextra.getBlock(local_pos + glm::ivec3(info[18], info[19], info[20]))].affectsAmbiantOcclusion;
    auto nb_hxhy = blocks_info[(int32_t)chunkextra.getBlock(local_pos + glm::ivec3(info[21], info[22], info[23]))].affectsAmbiantOcclusion;

    int32_t a00 = vertexAO(nb_lx, nb_ly, nb_lxly);
    int32_t a10 = vertexAO(nb_hx, nb_ly, nb_hxly);
    int32_t a11 = vertexAO(nb_hx, nb_hy, nb_hxhy);
    int32_t a01 = vertexAO(nb_lx, nb_hy, nb_lxhy);

    GLuint64 facedata = packVertex(x, y, z, orientation, texture_id, a00, a10, a11, a01);
    vertices.push_back(facedata);
}

ChunkRawMesh computeVertexBuffer(const glm::ivec3& chunk_pos)
{
    ChunkExtra chunkextra = ChunkExtra::get(chunk_pos);
    ChunkRawMesh chunk_raw_mesh;

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

        makeFace(chunk_raw_mesh.vertices, x, y, z, chunkextra, local_pos, Orientation::Front, texture_handle_lz);
        makeFace(chunk_raw_mesh.vertices, x, y, z, chunkextra, local_pos, Orientation::Back, texture_handle_hz);
        makeFace(chunk_raw_mesh.vertices, x, y, z, chunkextra, local_pos, Orientation::Bottom, texture_handle_ly);
        makeFace(chunk_raw_mesh.vertices, x, y, z, chunkextra, local_pos, Orientation::Top, texture_handle_hy);
        makeFace(chunk_raw_mesh.vertices, x, y, z, chunkextra, local_pos, Orientation::Left, texture_handle_lx);
        makeFace(chunk_raw_mesh.vertices, x, y, z, chunkextra, local_pos, Orientation::Right, texture_handle_hx);
    }
    }
    }

    return chunk_raw_mesh;
}

void ChunkMesh::updateVAO(
    BufferAllocator& buffer_allocator_vertices,
    const ChunkRawMesh& raw_mesh
){
    if (raw_mesh.vertices.size() == 0) {
        return;
    }

    const int32_t vertices_size = raw_mesh.vertices.size() * sizeof(GLuint64);

    slot_vertices = buffer_allocator_vertices.allocate(vertices_size, raw_mesh.vertices.data());
}
