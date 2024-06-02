#include "chunk.hpp"
#include "texture_manager.hpp"
#include "world.hpp"

#include <iostream>

int Chunk::XYZtoIndex(int x, int y, int z) {
    if (x < 0 || x > 15 || y < 0 || y > 15 || z < 0 || z > 15) return -1;
    return z * 16*16 + y * 16 + x;
}

void Chunk::computeChunckVAO(World &world, TextureManager &texture_manager)
{
    if (mesh.is_initialized == true) {
        mesh.delete_all();
    }

    /*
        position         float  32-bit  x 3
        uv               float  32-bit  x 2
        orientation      int    32-bit  x 1
        texture handle   float  32-bit  x 2
    */

    std::vector<GLuint64> textures_handles;
    std::vector<float> vertices;
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
        nb = world.get_block(world_pos + glm::ivec3(0, 0, -1));
        nbmtd = blocksMetadata[(int)nb];
        if (nbmtd.transparent) {
            vertices.insert(vertices.end(), {
                x+0.f, y+0.f, z+0.f, 0.f, 0.f, Orientation::Front,
                x+1.f, y+0.f, z+0.f, 1.f, 0.f, Orientation::Front,
                x+1.f, y+1.f, z+0.f, 1.f, 1.f, Orientation::Front,
                x+0.f, y+1.f, z+0.f, 0.f, 1.f, Orientation::Front,
            });

            ebo.insert(ebo.end(), {
                ebo_offset+0, ebo_offset+1, ebo_offset+2,
                ebo_offset+0, ebo_offset+2, ebo_offset+3
            });
            ebo_offset += 4;

            textures_handles.push_back(texture_side_handle);
        }

        // back
        nb = world.get_block(world_pos + glm::ivec3(0, 0, 1));
        nbmtd = blocksMetadata[(int)nb];
        if (nbmtd.transparent) {
            vertices.insert(vertices.end(), {
                x+0.f, y+0.f, z+1.f, 0.f, 0.f, Orientation::Back,
                x+1.f, y+1.f, z+1.f, 1.f, 1.f, Orientation::Back,
                x+1.f, y+0.f, z+1.f, 1.f, 0.f, Orientation::Back,
                x+0.f, y+1.f, z+1.f, 0.f, 1.f, Orientation::Back,
            });

            ebo.insert(ebo.end(), {
                ebo_offset+0, ebo_offset+1, ebo_offset+2,
                ebo_offset+0, ebo_offset+3, ebo_offset+1
            });
            ebo_offset += 4;

            textures_handles.push_back(texture_side_handle);
        }

        // down
        nb = world.get_block(world_pos + glm::ivec3(0, -1, 0));
        nbmtd = blocksMetadata[(int)nb];
        if (nbmtd.transparent) {
            vertices.insert(vertices.end(), {
                x+0.f, y+0.f, z+0.f, 0.f, 0.f, Orientation::Bottom,
                x+1.f, y+0.f, z+1.f, 1.f, 1.f, Orientation::Bottom,
                x+1.f, y+0.f, z+0.f, 1.f, 0.f, Orientation::Bottom,
                x+0.f, y+0.f, z+1.f, 0.f, 1.f, Orientation::Bottom,
            });
            textures_handles.push_back(texture_bot_handle);

            ebo.insert(ebo.end(), {
                ebo_offset+0, ebo_offset+1, ebo_offset+2,
                ebo_offset+0, ebo_offset+3, ebo_offset+1
            });
            ebo_offset += 4;
        }

        // top
        nb = world.get_block(world_pos + glm::ivec3(0, 1, 0));
        nbmtd = blocksMetadata[(int)nb];
        if (nbmtd.transparent) {
            vertices.insert(vertices.end(), {
                x+0.f, y+1.f, z+0.f, 0.f, 0.f, Orientation::Top,
                x+1.f, y+1.f, z+0.f, 1.f, 0.f, Orientation::Top,
                x+1.f, y+1.f, z+1.f, 1.f, 1.f, Orientation::Top,
                x+0.f, y+1.f, z+1.f, 0.f, 1.f, Orientation::Top,
            });
            textures_handles.push_back(texture_top_handle);

            ebo.insert(ebo.end(), {
                ebo_offset+0, ebo_offset+1, ebo_offset+2,
                ebo_offset+0, ebo_offset+2, ebo_offset+3
            });
            ebo_offset += 4;
        }

        // left
        nb = world.get_block(world_pos + glm::ivec3(-1, 0, 0));
        nbmtd = blocksMetadata[(int)nb];
        if (nbmtd.transparent) {
            vertices.insert(vertices.end(), {
                x+0.f, y+0.f, z+0.f, 0.f, 0.f, Orientation::Left,
                x+0.f, y+1.f, z+1.f, 1.f, 1.f, Orientation::Left,
                x+0.f, y+0.f, z+1.f, 1.f, 0.f, Orientation::Left,
                x+0.f, y+1.f, z+0.f, 0.f, 1.f, Orientation::Left,
            });
            textures_handles.push_back(texture_side_handle);

            ebo.insert(ebo.end(), {
                ebo_offset+0, ebo_offset+1, ebo_offset+2,
                ebo_offset+0, ebo_offset+3, ebo_offset+1
            });
            ebo_offset += 4;
        }

        // right
        nb = world.get_block(world_pos + glm::ivec3(1, 0, 0));
        nbmtd = blocksMetadata[(int)nb];
        if (nbmtd.transparent) {
            vertices.insert(vertices.end(), {
                x+1.f, y+0.f, z+0.f, 0.f, 0.f, Orientation::Right,
                x+1.f, y+0.f, z+1.f, 1.f, 0.f, Orientation::Right,
                x+1.f, y+1.f, z+1.f, 1.f, 1.f, Orientation::Right,
                x+1.f, y+1.f, z+0.f, 0.f, 1.f, Orientation::Right,
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

    mesh.is_initialized = true;

    glCreateVertexArrays(1, &mesh.VAO);
    glCreateBuffers(1, &mesh.VBO);
    glCreateBuffers(1, &mesh.EBO);
    glCreateBuffers(1, &mesh.ssbo_texture_handles);

    glNamedBufferData(mesh.VBO, vertices.size() * sizeof(GL_FLOAT), &vertices[0], GL_STATIC_DRAW);
    glNamedBufferStorage(mesh.EBO, ebo.size() * sizeof(GL_UNSIGNED_INT), &ebo[0], GL_DYNAMIC_STORAGE_BIT);

    glNamedBufferStorage(mesh.ssbo_texture_handles,
                            sizeof(GLuint64) * textures_handles.size(),
                            (const void *)textures_handles.data(),
                            GL_DYNAMIC_STORAGE_BIT);

    glEnableVertexArrayAttrib(mesh.VAO, 0);
    glVertexArrayAttribBinding(mesh.VAO, 0, 0);
    glVertexArrayAttribFormat(mesh.VAO, 0, 3, GL_FLOAT, GL_FALSE, 0);

    glEnableVertexArrayAttrib(mesh.VAO, 1);
    glVertexArrayAttribBinding(mesh.VAO, 1, 0);
    glVertexArrayAttribFormat(mesh.VAO, 1, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT));

    glEnableVertexArrayAttrib(mesh.VAO, 2);
    glVertexArrayAttribBinding(mesh.VAO, 2, 0);
    glVertexArrayAttribFormat(mesh.VAO, 2, 1, GL_FLOAT, GL_FALSE, 5 * sizeof(GL_FLOAT));

    glVertexArrayVertexBuffer(mesh.VAO, 0, mesh.VBO, 0, 6 * sizeof(GL_FLOAT));
    glVertexArrayElementBuffer(mesh.VAO, mesh.EBO);
}
