#pragma once

#include <unordered_map>
#include <map>
#include <math.h>

#include "glm/gtx/hash.hpp"
#include <glm/gtc/type_ptr.hpp>

#include "FastNoiseLite.h"

#include "View.hpp"
#include "Camera.hpp"
#include "Program.h"

#include "Texture.hpp"


enum Orientation : int {
    Top = 0,
    Bottom = 1,
    Front = 2,
    Back = 3,
    Left = 4,
    Right = 5,
};

enum BlockType : uint8_t {
    Air = 0,
    Grass = 1,
    Stone = 2,
};

enum Texture : int {
    GrassTop,
    GrassSide,
    GrassBottom,
};

std::unordered_map<Texture, const char* const> textures_name = {
    { Texture::GrassTop, "grass_block_top.png"},
    { Texture::GrassSide, "grass_block_side.png"},
    { Texture::GrassBottom, "dirt.png"},
};

std::unordered_map<BlockType, std::array<Texture, 3>> block_textures_path = {
    {
        BlockType::Grass, { Texture::GrassTop, Texture::GrassSide, Texture::GrassBottom }
    }
};

std::unordered_map<BlockType, std::array<GLuint64, 3>> block_textures_handles;

void loadAllTextures()
{
    std::string textures_path = "assets/textures/";

    for (const auto& [key, value] : block_textures_path) {
        // TODO: make texture manager to avoid duplicated when calling loadTextures

        GLuint texture_top = loadTexture((textures_path + textures_name[value[0]]).c_str(), GL_RGB, GL_NEAREST_MIPMAP_LINEAR, GL_NEAREST);
        GLuint texture_side = loadTexture((textures_path + textures_name[value[1]]).c_str(), GL_RGB, GL_NEAREST_MIPMAP_LINEAR, GL_NEAREST);
        GLuint texture_bot = loadTexture((textures_path + textures_name[value[2]]).c_str(), GL_RGB, GL_NEAREST_MIPMAP_LINEAR, GL_NEAREST);

        GLuint64 texture_top_handle = glGetTextureHandleARB(texture_top);
        GLuint64 texture_side_handle = glGetTextureHandleARB(texture_side);
        GLuint64 texture_bot_handle = glGetTextureHandleARB(texture_bot);

        glMakeTextureHandleResidentARB(texture_top_handle);
        glMakeTextureHandleResidentARB(texture_side_handle);
        glMakeTextureHandleResidentARB(texture_bot_handle);

        block_textures_handles.insert( {key, {texture_top_handle, texture_side_handle, texture_bot_handle}} );
    }
}

typedef struct Chunk
{
    glm::ivec3 pos;
    GLuint VAO;
    GLuint ssbo_texture_handles;
    uint vertices_count;
    BlockType blocks[4096]; // 16x16x16
} Chunk_t;

inline int XYZtoIndex(int x, int y, int z) {
    if (x < 0 || x > 15 || y < 0 || y > 15 || z < 0 || z > 15) return -1;
    return z * 16*16 + y * 16 + x;
}

void computeChunckVAO(Chunk_t &chunk)
{
    GLuint VBO;
    glCreateVertexArrays(1, &chunk.VAO);
    glCreateBuffers(1, &VBO);

    std::vector<GLuint64> textures_handles;
    glCreateBuffers(1, &chunk.ssbo_texture_handles);

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
        int index = z * 16*16 + y * 16 + x;
        if (chunk.blocks[index] == BlockType::Air) continue;

        auto [texture_top_handle, texture_side_handle, texture_bot_handle] = block_textures_handles[chunk.blocks[index]];

        // front
        i = XYZtoIndex(x, y, z-1);
        if (i == -1 || chunk.blocks[i] == BlockType::Air) {
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
        i = XYZtoIndex(x, y, z+1);
        if (i == -1 || chunk.blocks[i] == BlockType::Air) {
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
        i = XYZtoIndex(x, y-1, z);
        if (i == -1 || chunk.blocks[i] == BlockType::Air) {
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
        i = XYZtoIndex(x, y+1, z);
        if (i == -1 || chunk.blocks[i] == BlockType::Air) {
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
        i = XYZtoIndex(x-1, y, z);
        if (i == -1 || chunk.blocks[i] == BlockType::Air) {
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
        i = XYZtoIndex(x+1, y, z);
        if (i == -1 || chunk.blocks[i] == BlockType::Air) {
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

    chunk.vertices_count = v.size() / 5;

    // std::cout << chunk.vertices_count << std::endl;

    glNamedBufferData(VBO, v.size() * sizeof(GL_FLOAT), &v[0], GL_STATIC_DRAW);

    glEnableVertexArrayAttrib(chunk.VAO, 0);
    glVertexArrayAttribBinding(chunk.VAO, 0, 0);
    glVertexArrayAttribFormat(chunk.VAO, 0, 3, GL_FLOAT, GL_FALSE, 0);

    glEnableVertexArrayAttrib(chunk.VAO, 1);
    glVertexArrayAttribBinding(chunk.VAO, 1, 0);
    glVertexArrayAttribFormat(chunk.VAO, 1, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT));

    glEnableVertexArrayAttrib(chunk.VAO, 2);
    glVertexArrayAttribBinding(chunk.VAO, 2, 0);
    glVertexArrayAttribFormat(chunk.VAO, 2, 1, GL_FLOAT, GL_FALSE, 5 * sizeof(GL_FLOAT));

    glVertexArrayVertexBuffer(chunk.VAO, 0, VBO, 0, 6 * sizeof(GL_FLOAT));

    glNamedBufferStorage(chunk.ssbo_texture_handles,
                     sizeof(GLuint64) * textures_handles.size(),
                     (const void *)textures_handles.data(),
                     GL_DYNAMIC_STORAGE_BIT);
}

class GameView: public View {
    public:
        GameView(Context& ctx): View(ctx)
        {
            int width, height;
            glfwGetWindowSize(ctx.window, &width, &height);

            camera = new OrbitCamera(
                glm::vec3(0.0f), M_PI/4, M_PI/4, 20.0f,
                60.0f, (float)width / (float)height, 0.1f, 1000.0f
            );

            noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);

            loadAllTextures();

            cube_shader = new Program("./assets/shaders/cube.vs", "./assets/shaders/cube.fs");

            int SIZE_X = 4;
            int SIZE_Y = 1;
            int SIZE_Z = 4;
            for (int x = -SIZE_X ; x < SIZE_X ; ++x) {
            for (int y = -SIZE_Y ; y < SIZE_Y ; ++y) {
            for (int z = -SIZE_Z ; z < SIZE_Z ; ++z) {
                Chunk c = generateChunk({x, y, z});
                chunks[c.pos] = c;
            }
            }
            }

        }

        Chunk_t generateChunk(glm::ivec3 pos)
        {
            Chunk_t chunk;
            chunk.pos = pos;

            // srand(time(NULL));
            glm::vec3 chunkPosWorld = pos * 16;

            for (int z = 0 ; z < 16 ; ++z) {
            for (int x = 0 ; x < 16 ; ++x) {
                int height = 1.0f + (noise.GetNoise(chunkPosWorld.x + x, chunkPosWorld.z + z) * 0.5f + 0.5f) * 15.0f;

                for (int y = 0 ; y < 16 ; ++y) {
                    int index = z * 16*16 + y * 16 + x;

                    // chunk.blocks[index] = BlockType::Grass;
                    chunk.blocks[index] = chunkPosWorld.y + y < height ? BlockType::Grass : BlockType::Air;
                    // chunk.blocks[index] = rand() % 3 == 0 ? BlockType::Grass : BlockType::Air;
                }
            }
            }

            computeChunckVAO(chunk);

            return chunk;
        }

        void onUpdate(float time_since_start, float dt)
        {

        }

        void onDraw(float time_since_start, float dt)
        {
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);

            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
            glFrontFace(GL_CW);

            glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            cube_shader->use();
            cube_shader->setMat4("u_projectionMatrix", camera->getProjection());
            cube_shader->setMat4("u_viewMatrix", camera->getView());

            for (const auto& [key, chunk] : chunks)
            {
                cube_shader->setVec3("u_chunkPos", chunk.pos * 16);
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, chunk.ssbo_texture_handles);
                glBindVertexArray(chunk.VAO);
                glDrawArrays(GL_TRIANGLES, 0, chunk.vertices_count);
            }

            gui(dt);
        }

        void gui(float dt)
        {
            ctx.imguiNewFrame();
            ImGui::Begin("Debug");

            ImGui::Text("%.4f ms", dt);
            ImGui::Text("%.2f fps", 1.0f / dt);

            glm::vec3 camera_pos = camera->getPosition();
            ImGui::Text("center: %.2f, %.2f, %.2f", camera_pos.x, camera_pos.y, camera_pos.z);
            ImGui::Text("yaw: %.2f", camera->getYaw());
            ImGui::Text("pitch: %.2f", camera->getPitch());

            ImGui::End();
            ctx.imguiRender();
        }

        void onKeyPress(int key)
        {
        }

        void onKeyRelease(int key)
        {
        }

        void onMouseDrag(int x, int y, int dx, int dy)
        {
            if (ImGui::GetIO().WantCaptureMouse) return;

            camera->setYaw( camera->getYaw() - (dx * 0.005f) );
            camera->setPitch( camera->getPitch() + (dy * 0.005f) );
        }

        void onMouseScroll(int scroll_x, int scroll_y)
        {
            float scale = 1.0f;
            if (ctx.keyState[GLFW_KEY_LEFT_SHIFT] == GLFW_PRESS)
                scale = 8.0f;

            camera->setDistance( camera->getDistance() - (scroll_y * scale) );
        }

        void onResize(int width, int height)
        {
            glViewport(0, 0, width, height);
        }

    private:
        OrbitCamera* camera;
        Program* cube_shader;

        std::unordered_map<glm::ivec3, Chunk_t> chunks;

        FastNoiseLite noise;
};
