#include <iostream>

#include "chunk.hpp"
#include "texture_manager.hpp"
#include "world.hpp"
#include "VAO.hpp"

int Chunk::XYZtoIndex(int x, int y, int z) {
    if (x < 0 || x > 15 || y < 0 || y > 15 || z < 0 || z > 15) return -1;
    return z * 16*16 + y * 16 + x;
}

GLuint packVertex(int x, int y, int z, int u, int v, int o, int t) {
    // TODO: add texture id in there
    // 4 bytes, 32 bits
    // 00000000000000000000000000000000
    //     ttttttttooouvzzzzzyyyyyxxxxx
    GLuint p =
        ((x & 31)   << 0)   |
        ((y & 31)   << 5)   |
        ((z & 31)   << 10)  |
        ((u & 1)    << 15)  |
        ((v & 1)    << 16)  |
        ((o & 7)    << 17)  |
        ((t & 255)  << 20);

    return p;
}

void Chunk::computeVertexBuffer(const World &world, const TextureManager &texture_manager)
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

    ChunkExtra chunkextra = ChunkExtra::get(world, pos);

    mesh.vertices.clear();
    mesh.ebo.clear();

    GLuint ebo_offset = 0;

    for (int z = 0 ; z < 16 ; ++z) {
    for (int y = 0 ; y < 16 ; ++y) {
    for (int x = 0 ; x < 16 ; ++x) {
        const int index = z * 16*16 + y * 16 + x;
        const BlockType block = blocks[index];

        if (block == BlockType::Air) continue;

        // const BlockMetadata block_metadata = blocksMetadata[(int)block];
        const glm::ivec3 local_pos = glm::ivec3(x, y, z);
        // const glm::ivec3 world_pos = (pos * 16) + local_pos;
        auto [texture_top_handle, texture_side_handle, texture_bot_handle] = texture_manager.block_textures_ids[block];

        BlockType nb; // neighbour block
        BlockMetadata nbmtd; // neighbour block metadata

        // front
        // nb = world.getBlock(world_pos + glm::ivec3(0, 0, -1));
        nb = chunkextra.getBlock(local_pos + glm::ivec3(0, 0, -1));
        nbmtd = blocksMetadata[(int)nb];
        if (nbmtd.transparent) {
            mesh.vertices.insert(mesh.vertices.end(), {
                packVertex(x+0, y+0, z+0, 0, 0, Orientation::Front, texture_side_handle),
                packVertex(x+1, y+0, z+0, 1, 0, Orientation::Front, texture_side_handle),
                packVertex(x+1, y+1, z+0, 1, 1, Orientation::Front, texture_side_handle),
                packVertex(x+0, y+1, z+0, 0, 1, Orientation::Front, texture_side_handle),
            });

            mesh.ebo.insert(mesh.ebo.end(), {
                ebo_offset+0, ebo_offset+1, ebo_offset+2,
                ebo_offset+0, ebo_offset+2, ebo_offset+3
            });
            ebo_offset += 4;
        }

        // back
        // nb = world.getBlock(world_pos + glm::ivec3(0, 0, 1));
        nb = chunkextra.getBlock(local_pos + glm::ivec3(0, 0, 1));
        nbmtd = blocksMetadata[(int)nb];
        if (nbmtd.transparent) {
            mesh.vertices.insert(mesh.vertices.end(), {
                packVertex(x+0, y+0, z+1, 0, 0, Orientation::Back, texture_side_handle),
                packVertex(x+1, y+1, z+1, 1, 1, Orientation::Back, texture_side_handle),
                packVertex(x+1, y+0, z+1, 1, 0, Orientation::Back, texture_side_handle),
                packVertex(x+0, y+1, z+1, 0, 1, Orientation::Back, texture_side_handle),
            });

            mesh.ebo.insert(mesh.ebo.end(), {
                ebo_offset+0, ebo_offset+1, ebo_offset+2,
                ebo_offset+0, ebo_offset+3, ebo_offset+1
            });
            ebo_offset += 4;
        }

        // down
        // nb = world.getBlock(world_pos + glm::ivec3(0, -1, 0));
        nb = chunkextra.getBlock(local_pos + glm::ivec3(0, -1, 0));
        nbmtd = blocksMetadata[(int)nb];
        if (nbmtd.transparent) {
            mesh.vertices.insert(mesh.vertices.end(), {
                packVertex(x+0, y+0, z+0, 0, 0, Orientation::Bottom, texture_bot_handle),
                packVertex(x+1, y+0, z+1, 1, 1, Orientation::Bottom, texture_bot_handle),
                packVertex(x+1, y+0, z+0, 1, 0, Orientation::Bottom, texture_bot_handle),
                packVertex(x+0, y+0, z+1, 0, 1, Orientation::Bottom, texture_bot_handle),
            });

            mesh.ebo.insert(mesh.ebo.end(), {
                ebo_offset+0, ebo_offset+1, ebo_offset+2,
                ebo_offset+0, ebo_offset+3, ebo_offset+1
            });
            ebo_offset += 4;
        }

        // top
        // nb = world.getBlock(world_pos + glm::ivec3(0, 1, 0));
        nb = chunkextra.getBlock(local_pos + glm::ivec3(0, 1, 0));
        nbmtd = blocksMetadata[(int)nb];
        if (nbmtd.transparent) {
            mesh.vertices.insert(mesh.vertices.end(), {
                packVertex(x+0, y+1, z+0, 0, 0, Orientation::Top, texture_top_handle),
                packVertex(x+1, y+1, z+0, 1, 0, Orientation::Top, texture_top_handle),
                packVertex(x+1, y+1, z+1, 1, 1, Orientation::Top, texture_top_handle),
                packVertex(x+0, y+1, z+1, 0, 1, Orientation::Top, texture_top_handle),
            });

            mesh.ebo.insert(mesh.ebo.end(), {
                ebo_offset+0, ebo_offset+1, ebo_offset+2,
                ebo_offset+0, ebo_offset+2, ebo_offset+3
            });
            ebo_offset += 4;
        }

        // left
        // nb = world.getBlock(world_pos + glm::ivec3(-1, 0, 0));
        nb = chunkextra.getBlock(local_pos + glm::ivec3(-1, 0, 0));
        nbmtd = blocksMetadata[(int)nb];
        if (nbmtd.transparent) {
            mesh.vertices.insert(mesh.vertices.end(), {
                packVertex(x+0, y+0, z+0, 0, 0, Orientation::Left, texture_side_handle),
                packVertex(x+0, y+1, z+1, 1, 1, Orientation::Left, texture_side_handle),
                packVertex(x+0, y+0, z+1, 1, 0, Orientation::Left, texture_side_handle),
                packVertex(x+0, y+1, z+0, 0, 1, Orientation::Left, texture_side_handle),
            });

            mesh.ebo.insert(mesh.ebo.end(), {
                ebo_offset+0, ebo_offset+1, ebo_offset+2,
                ebo_offset+0, ebo_offset+3, ebo_offset+1
            });
            ebo_offset += 4;
        }

        // right
        // nb = world.getBlock(world_pos + glm::ivec3(1, 0, 0));
        nb = chunkextra.getBlock(local_pos + glm::ivec3(1, 0, 0));
        nbmtd = blocksMetadata[(int)nb];
        if (nbmtd.transparent) {
            mesh.vertices.insert(mesh.vertices.end(), {
                packVertex(x+1, y+0, z+0, 0, 0, Orientation::Right, texture_side_handle),
                packVertex(x+1, y+0, z+1, 1, 0, Orientation::Right, texture_side_handle),
                packVertex(x+1, y+1, z+1, 1, 1, Orientation::Right, texture_side_handle),
                packVertex(x+1, y+1, z+0, 0, 1, Orientation::Right, texture_side_handle),
            });

            mesh.ebo.insert(mesh.ebo.end(), {
                ebo_offset+0, ebo_offset+1, ebo_offset+2,
                ebo_offset+0, ebo_offset+2, ebo_offset+3
            });
            ebo_offset += 4;
        }

    }
    }
    }

    mesh.indices_count = mesh.ebo.size();
}

void Chunk::updateVAO()
{
    if (mesh.VAO == 0) {
        glCreateVertexArrays(1, &mesh.VAO);
        glCreateBuffers(1, &mesh.VBO);
        glCreateBuffers(1, &mesh.EBO);

        glEnableVertexArrayAttrib(mesh.VAO, 0);
        glVertexArrayAttribBinding(mesh.VAO, 0, 0);
        glVertexArrayAttribIFormat(mesh.VAO, 0, 1, GL_UNSIGNED_INT, 0);

        glVertexArrayVertexBuffer(mesh.VAO, 0, mesh.VBO, 0, 1 * sizeof(GLuint));
        glVertexArrayElementBuffer(mesh.VAO, mesh.EBO);
    }

    glNamedBufferData(mesh.VBO, mesh.vertices.size() * sizeof(GLuint), &mesh.vertices[0], GL_STATIC_DRAW);
    glNamedBufferData(mesh.EBO, mesh.ebo.size() * sizeof(GLuint), &mesh.ebo[0], GL_STATIC_DRAW);

    // TODO: clear() vertices et ebo vectors if not needed anymore
    // mesh.vertices.clear();
    // mesh.ebo.clear();

    // mesh.vertices.shrink_to_fit();
    // mesh.ebo.shrink_to_fit();
}
