#include <iostream>

#include "chunk.hpp"
#include "texture_manager.hpp"
#include "world.hpp"
#include "VAO.hpp"

int Chunk::XYZtoIndex(int x, int y, int z) {
    if (x < 0 || x > 15 || y < 0 || y > 15 || z < 0 || z > 15) return -1;
    return z * 16*16 + y * 16 + x;
}

GLuint packVertex(int x, int y, int z, int u, int v, int o) {
    // TODO: add texture id in there
    // 4 bytes, 32 bits
    // 00000000000000000000000000000000
    //             ooouvzzzzzyyyyyxxxxx
    GLuint p =
        ((x & 31)   << 0)   |
        ((y & 31)   << 5)   |
        ((z & 31)   << 10)  |
        ((u & 1)    << 15)  |
        ((v & 1)    << 16)  |
        ((o & 7)    << 17);

    return p;
}

void Chunk::computeChunckVAO(World &world, TextureManager &texture_manager)
{
    // TODO: check if chunk is only air, then remove it from world

    /*
        position         float  32-bit  x 3
        uv               float  32-bit  x 2
        orientation      int    32-bit  x 1
    */

    std::vector<GLuint64> textures_handles;
    std::vector<GLuint> vertices;
    std::vector<GLuint> ebo;
    GLuint ebo_offset = 0;

    for (int z = 0 ; z < 16 ; ++z) {
    for (int y = 0 ; y < 16 ; ++y) {
    for (int x = 0 ; x < 16 ; ++x) {
        const int index = z * 16*16 + y * 16 + x;
        const BlockType block = blocks[index];

        if (block == BlockType::Air) continue;

        // const BlockMetadata block_metadata = blocksMetadata[(int)block];
        const glm::ivec3 world_pos = (pos * 16) + glm::ivec3(x, y, z);
        auto [texture_top_handle, texture_side_handle, texture_bot_handle] = texture_manager.block_textures_handles[block];

        BlockType nb; // neighbour block
        BlockMetadata nbmtd; // neighbour block metadata

        // front
        nb = world.getBlock(world_pos + glm::ivec3(0, 0, -1));
        nbmtd = blocksMetadata[(int)nb];
        if (nbmtd.transparent) {
            vertices.insert(vertices.end(), {
                packVertex(x+0, y+0, z+0, 0, 0, Orientation::Front),
                packVertex(x+1, y+0, z+0, 1, 0, Orientation::Front),
                packVertex(x+1, y+1, z+0, 1, 1, Orientation::Front),
                packVertex(x+0, y+1, z+0, 0, 1, Orientation::Front),
            });

            ebo.insert(ebo.end(), {
                ebo_offset+0, ebo_offset+1, ebo_offset+2,
                ebo_offset+0, ebo_offset+2, ebo_offset+3
            });
            ebo_offset += 4;

            textures_handles.push_back(texture_side_handle);
        }

        // back
        nb = world.getBlock(world_pos + glm::ivec3(0, 0, 1));
        nbmtd = blocksMetadata[(int)nb];
        if (nbmtd.transparent) {
            vertices.insert(vertices.end(), {
                packVertex(x+0, y+0, z+1, 0, 0, Orientation::Back),
                packVertex(x+1, y+1, z+1, 1, 1, Orientation::Back),
                packVertex(x+1, y+0, z+1, 1, 0, Orientation::Back),
                packVertex(x+0, y+1, z+1, 0, 1, Orientation::Back),
            });

            ebo.insert(ebo.end(), {
                ebo_offset+0, ebo_offset+1, ebo_offset+2,
                ebo_offset+0, ebo_offset+3, ebo_offset+1
            });
            ebo_offset += 4;

            textures_handles.push_back(texture_side_handle);
        }

        // down
        nb = world.getBlock(world_pos + glm::ivec3(0, -1, 0));
        nbmtd = blocksMetadata[(int)nb];
        if (nbmtd.transparent) {
            vertices.insert(vertices.end(), {
                packVertex(x+0, y+0, z+0, 0, 0, Orientation::Bottom),
                packVertex(x+1, y+0, z+1, 1, 1, Orientation::Bottom),
                packVertex(x+1, y+0, z+0, 1, 0, Orientation::Bottom),
                packVertex(x+0, y+0, z+1, 0, 1, Orientation::Bottom),
            });
            textures_handles.push_back(texture_bot_handle);

            ebo.insert(ebo.end(), {
                ebo_offset+0, ebo_offset+1, ebo_offset+2,
                ebo_offset+0, ebo_offset+3, ebo_offset+1
            });
            ebo_offset += 4;
        }

        // top
        nb = world.getBlock(world_pos + glm::ivec3(0, 1, 0));
        nbmtd = blocksMetadata[(int)nb];
        if (nbmtd.transparent) {
            vertices.insert(vertices.end(), {
                packVertex(x+0, y+1, z+0, 0, 0, Orientation::Top),
                packVertex(x+1, y+1, z+0, 1, 0, Orientation::Top),
                packVertex(x+1, y+1, z+1, 1, 1, Orientation::Top),
                packVertex(x+0, y+1, z+1, 0, 1, Orientation::Top),
            });
            textures_handles.push_back(texture_top_handle);

            ebo.insert(ebo.end(), {
                ebo_offset+0, ebo_offset+1, ebo_offset+2,
                ebo_offset+0, ebo_offset+2, ebo_offset+3
            });
            ebo_offset += 4;
        }

        // left
        nb = world.getBlock(world_pos + glm::ivec3(-1, 0, 0));
        nbmtd = blocksMetadata[(int)nb];
        if (nbmtd.transparent) {
            vertices.insert(vertices.end(), {
                packVertex(x+0, y+0, z+0, 0, 0, Orientation::Left),
                packVertex(x+0, y+1, z+1, 1, 1, Orientation::Left),
                packVertex(x+0, y+0, z+1, 1, 0, Orientation::Left),
                packVertex(x+0, y+1, z+0, 0, 1, Orientation::Left),
            });
            textures_handles.push_back(texture_side_handle);

            ebo.insert(ebo.end(), {
                ebo_offset+0, ebo_offset+1, ebo_offset+2,
                ebo_offset+0, ebo_offset+3, ebo_offset+1
            });
            ebo_offset += 4;
        }

        // right
        nb = world.getBlock(world_pos + glm::ivec3(1, 0, 0));
        nbmtd = blocksMetadata[(int)nb];
        if (nbmtd.transparent) {
            vertices.insert(vertices.end(), {
                packVertex(x+1, y+0, z+0, 0, 0, Orientation::Right),
                packVertex(x+1, y+0, z+1, 1, 0, Orientation::Right),
                packVertex(x+1, y+1, z+1, 1, 1, Orientation::Right),
                packVertex(x+1, y+1, z+0, 0, 1, Orientation::Right),
            });
            textures_handles.push_back(texture_side_handle);

            ebo.insert(ebo.end(), {
                ebo_offset+0, ebo_offset+1, ebo_offset+2,
                ebo_offset+0, ebo_offset+2, ebo_offset+3
            });
            ebo_offset += 4;
        }

    }
    }
    }

    mesh.indices_count = ebo.size();
    if (mesh.indices_count == 0) return;

    // mesh.VBO = createBuffer(&vertices[0], vertices.size() * sizeof(GLfloat), GL_DYNAMIC_STORAGE_BIT);
    // mesh.EBO = createBuffer(&ebo[0], ebo.size() * sizeof(GL_UNSIGNED_INT), GL_DYNAMIC_STORAGE_BIT);
    // mesh.ssbo_texture_handles = createBuffer((const void *)textures_handles.data(), sizeof(GLuint64) * textures_handles.size(), GL_DYNAMIC_STORAGE_BIT);
    // mesh.VAO = createVAO(mesh.VBO, "3f 2f 1i", mesh.EBO);

    // Note: glNamedBufferStorage are immutable (can't be resized)
    if (mesh.VAO == 0) {
        glCreateVertexArrays(1, &mesh.VAO);
        glCreateBuffers(1, &mesh.VBO);
        glCreateBuffers(1, &mesh.EBO);
        glCreateBuffers(1, &mesh.ssbo_texture_handles);

        glEnableVertexArrayAttrib(mesh.VAO, 0);
        glVertexArrayAttribBinding(mesh.VAO, 0, 0);
        glVertexArrayAttribIFormat(mesh.VAO, 0, 1, GL_UNSIGNED_INT, 0);

        // glEnableVertexArrayAttrib(mesh.VAO, 0);
        // glVertexArrayAttribBinding(mesh.VAO, 0, 0);
        // glVertexArrayAttribIFormat(mesh.VAO, 0, 3, GL_INT, 0 * sizeof(GLint));

        // glEnableVertexArrayAttrib(mesh.VAO, 1);
        // glVertexArrayAttribBinding(mesh.VAO, 1, 0);
        // glVertexArrayAttribIFormat(mesh.VAO, 1, 2, GL_INT, 3 * sizeof(GLint));

        // glEnableVertexArrayAttrib(mesh.VAO, 2);
        // glVertexArrayAttribBinding(mesh.VAO, 2, 0);
        // glVertexArrayAttribIFormat(mesh.VAO, 2, 1, GL_INT, 5 * sizeof(GLint));

        // glVertexArrayVertexBuffer(mesh.VAO, 0, mesh.VBO, 0, 6 * sizeof(GLint));
        glVertexArrayVertexBuffer(mesh.VAO, 0, mesh.VBO, 0, 1 * sizeof(GLuint));
        glVertexArrayElementBuffer(mesh.VAO, mesh.EBO);
    }

    glNamedBufferData(mesh.VBO, vertices.size() * sizeof(GLuint), &vertices[0], GL_STATIC_DRAW);
    glNamedBufferData(mesh.EBO, ebo.size() * sizeof(GLuint), &ebo[0], GL_STATIC_DRAW);
    glNamedBufferData(mesh.ssbo_texture_handles, sizeof(GLuint64) * textures_handles.size(), (const void *)textures_handles.data(), GL_STATIC_DRAW);
}
