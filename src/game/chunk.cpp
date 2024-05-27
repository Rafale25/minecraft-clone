#include "chunk.hpp"
#include "texture_manager.hpp"
#include "world.hpp"

int Chunk::XYZtoIndex(int x, int y, int z) {
    if (x < 0 || x > 15 || y < 0 || y > 15 || z < 0 || z > 15) return -1;
    return z * 16*16 + y * 16 + x;
}

void Chunk::computeChunckVAO(World &world, TextureManager &texture_manager)
{
    // vao_initialized = true;

    GLuint VBO;
    glCreateVertexArrays(1, &VAO);
    glCreateBuffers(1, &VBO);

    std::vector<GLuint64> textures_handles;
    glCreateBuffers(1, &ssbo_texture_handles);

    /*
        position         float  32-bit  x 3
        uv               float  32-bit  x 2
        orientation      int    32-bit  x 1
        texture handle   float  32-bit  x 2
    */

    std::vector<float> v;
    int i;

    for (int z = 0 ; z < 16 ; ++z) {
    for (int y = 0 ; y < 16 ; ++y) {
    for (int x = 0 ; x < 16 ; ++x) {
        glm::ivec3 world_pos = (pos * 16) + glm::ivec3(x, y, z);
        int index = z * 16*16 + y * 16 + x;
        if (blocks[index] == BlockType::Air) continue;

        auto [texture_top_handle, texture_side_handle, texture_bot_handle] = texture_manager.block_textures_handles[blocks[index]];

        // front
        if (world.get_block(world_pos + glm::ivec3(0, 0, -1)) == BlockType::Air) {
            v.insert(v.end(), {
                x+0.f, y+0.f, z+0.f, 0.f, 0.f, Orientation::Front,
                x+1.f, y+0.f, z+0.f, 1.f, 0.f, Orientation::Front,
                x+1.f, y+1.f, z+0.f, 1.f, 1.f, Orientation::Front,

                x+0.f, y+0.f, z+0.f, 0.f, 0.f, Orientation::Front,
                x+1.f, y+1.f, z+0.f, 1.f, 1.f, Orientation::Front,
                x+0.f, y+1.f, z+0.f, 0.f, 1.f, Orientation::Front,
            });
            textures_handles.push_back(texture_side_handle);
        }

        // back
        if (world.get_block(world_pos + glm::ivec3(0, 0, 1)) == BlockType::Air) {
            v.insert(v.end(), {
                x+0.f, y+0.f, z+1.f, 0.f, 0.f, Orientation::Back,
                x+1.f, y+1.f, z+1.f, 1.f, 1.f, Orientation::Back,
                x+1.f, y+0.f, z+1.f, 1.f, 0.f, Orientation::Back,

                x+0.f, y+0.f, z+1.f, 0.f, 0.f, Orientation::Back,
                x+0.f, y+1.f, z+1.f, 0.f, 1.f, Orientation::Back,
                x+1.f, y+1.f, z+1.f, 1.f, 1.f, Orientation::Back,
            });
            textures_handles.push_back(texture_side_handle);
        }

        // down
        if (world.get_block(world_pos + glm::ivec3(0, -1, 0)) == BlockType::Air) {
            v.insert(v.end(), {
                x+0.f, y+0.f, z+0.f, 0.f, 0.f, Orientation::Bottom,
                x+1.f, y+0.f, z+1.f, 1.f, 1.f, Orientation::Bottom,
                x+1.f, y+0.f, z+0.f, 1.f, 0.f, Orientation::Bottom,

                x+0.f, y+0.f, z+0.f, 0.f, 0.f, Orientation::Bottom,
                x+0.f, y+0.f, z+1.f, 0.f, 1.f, Orientation::Bottom,
                x+1.f, y+0.f, z+1.f, 1.f, 1.f, Orientation::Bottom,
            });
            textures_handles.push_back(texture_bot_handle);
        }

        // top
        if (world.get_block(world_pos + glm::ivec3(0, 1, 0)) == BlockType::Air) {
            v.insert(v.end(), {
                x+0.f, y+1.f, z+0.f, 0.f, 0.f, Orientation::Top,
                x+1.f, y+1.f, z+0.f, 1.f, 0.f, Orientation::Top,
                x+1.f, y+1.f, z+1.f, 1.f, 1.f, Orientation::Top,

                x+0.f, y+1.f, z+0.f, 0.f, 0.f, Orientation::Top,
                x+1.f, y+1.f, z+1.f, 1.f, 1.f, Orientation::Top,
                x+0.f, y+1.f, z+1.f, 0.f, 1.f, Orientation::Top,
            });
            textures_handles.push_back(texture_top_handle);
        }


        // left
        if (world.get_block(world_pos + glm::ivec3(-1, 0, 0)) == BlockType::Air) {
            v.insert(v.end(), {
                x+0.f, y+0.f, z+0.f, 0.f, 0.f, Orientation::Left,
                x+0.f, y+1.f, z+1.f, 1.f, 1.f, Orientation::Left,
                x+0.f, y+0.f, z+1.f, 1.f, 0.f, Orientation::Left,

                x+0.f, y+0.f, z+0.f, 0.f, 0.f, Orientation::Left,
                x+0.f, y+1.f, z+0.f, 0.f, 1.f, Orientation::Left,
                x+0.f, y+1.f, z+1.f, 1.f, 1.f, Orientation::Left,
            });
            textures_handles.push_back(texture_side_handle);
        }

        // right
        if (world.get_block(world_pos + glm::ivec3(1, 0, 0)) == BlockType::Air) {
            v.insert(v.end(), {
                x+1.f, y+0.f, z+0.f, 0.f, 0.f, Orientation::Right,
                x+1.f, y+0.f, z+1.f, 1.f, 0.f, Orientation::Right,
                x+1.f, y+1.f, z+1.f, 1.f, 1.f, Orientation::Right,

                x+1.f, y+0.f, z+0.f, 0.f, 0.f, Orientation::Right,
                x+1.f, y+1.f, z+1.f, 1.f, 1.f, Orientation::Right,
                x+1.f, y+1.f, z+0.f, 0.f, 1.f, Orientation::Right,
            });
            textures_handles.push_back(texture_side_handle);
        }
    }
    }
    }

    vertices_count = v.size() / 6;

    // std::cout << _vertices_count << std::endl;

    glNamedBufferData(VBO, v.size() * sizeof(GL_FLOAT), &v[0], GL_STATIC_DRAW);

    glEnableVertexArrayAttrib(VAO, 0);
    glVertexArrayAttribBinding(VAO, 0, 0);
    glVertexArrayAttribFormat(VAO, 0, 3, GL_FLOAT, GL_FALSE, 0);

    glEnableVertexArrayAttrib(VAO, 1);
    glVertexArrayAttribBinding(VAO, 1, 0);
    glVertexArrayAttribFormat(VAO, 1, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT));

    glEnableVertexArrayAttrib(VAO, 2);
    glVertexArrayAttribBinding(VAO, 2, 0);
    glVertexArrayAttribFormat(VAO, 2, 1, GL_FLOAT, GL_FALSE, 5 * sizeof(GL_FLOAT));

    glVertexArrayVertexBuffer(VAO, 0, VBO, 0, 6 * sizeof(GL_FLOAT));

    glNamedBufferStorage(ssbo_texture_handles, // WTF: TODO DONT DUPLICATE THE TEXTURE HANDLES FOR EVERY CHUNKS
                     sizeof(GLuint64) * textures_handles.size(),
                     (const void *)textures_handles.data(),
                     GL_DYNAMIC_STORAGE_BIT);
}
