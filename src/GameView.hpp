#pragma once

#include <unordered_map>
#include <map>

#ifndef GLM_ENABLE_EXPERIMENTAL
#define GLM_ENABLE_EXPERIMENTAL
#endif

#include "glm/gtx/hash.hpp"
#include <glm/gtc/type_ptr.hpp>

#include "View.hpp"
#include "Camera.hpp"
#include "Program.h"

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
};

enum Materials : int {
    Grass_Top,
    Grass_Side,
    Grass_Bottom
};

typedef struct Chunk
{
    glm::ivec2 pos;
    GLuint VAO;
    uint vertices_count;
    BlockType blocks[4096]; // 16 x 16 x 16
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

    /*
        position     float  32-bit  x 3
        uv           float  32-bit  x 2
        orientation  int    32-bit  x 1
        material     int    32-bit  x 1
    */

    std::vector<float> v;
    int i;

    for (int z = 0 ; z < 16 ; ++z) {
    for (int y = 0 ; y < 16 ; ++y) {
    for (int x = 0 ; x < 16 ; ++x) {
        int index = z * 16*16 + y * 16 + x;
        if (chunk.blocks[index] == BlockType::Air) continue;

        // front
        i = XYZtoIndex(x, y, z-1);
        if (i == -1 || chunk.blocks[i] == BlockType::Air)
            v.insert(v.end(), {
                x+0.f, y+0.f, z+0.f, 0.f, 0.f, Orientation::Front,
                x+1.f, y+0.f, z+0.f, 1.f, 0.f, Orientation::Front,
                x+1.f, y+1.f, z+0.f, 1.f, 1.f, Orientation::Front,

                x+0.f, y+0.f, z+0.f, 0.f, 0.f, Orientation::Front,
                x+1.f, y+1.f, z+0.f, 1.f, 1.f, Orientation::Front,
                x+0.f, y+1.f, z+0.f, 0.f, 1.f, Orientation::Front,
            });

        // back
        i = XYZtoIndex(x, y, z+1);
        if (i == -1 || chunk.blocks[i] == BlockType::Air)
            v.insert(v.end(), {
                x+0.f, y+0.f, z+1.f, 0.f, 0.f, Orientation::Back,
                x+1.f, y+1.f, z+1.f, 1.f, 1.f, Orientation::Back,
                x+1.f, y+0.f, z+1.f, 1.f, 0.f, Orientation::Back,

                x+0.f, y+0.f, z+1.f, 0.f, 0.f, Orientation::Back,
                x+0.f, y+1.f, z+1.f, 0.f, 1.f, Orientation::Back,
                x+1.f, y+1.f, z+1.f, 1.f, 1.f, Orientation::Back,
            });

        // down
        i = XYZtoIndex(x, y-1, z);
        if (i == -1 || chunk.blocks[i] == BlockType::Air)
            v.insert(v.end(), {
                x+0.f, y+0.f, z+0.f, 0.f, 0.f, Orientation::Bottom,
                x+1.f, y+0.f, z+1.f, 1.f, 1.f, Orientation::Bottom,
                x+1.f, y+0.f, z+0.f, 1.f, 0.f, Orientation::Bottom,

                x+0.f, y+0.f, z+0.f, 0.f, 0.f, Orientation::Bottom,
                x+0.f, y+0.f, z+1.f, 0.f, 1.f, Orientation::Bottom,
                x+1.f, y+0.f, z+1.f, 1.f, 1.f, Orientation::Bottom,
            });

        // top
        i = XYZtoIndex(x, y+1, z);
        if (i == -1 || chunk.blocks[i] == BlockType::Air)
            v.insert(v.end(), {
                x+0.f, y+1.f, z+0.f, 0.f, 0.f, Orientation::Top,
                x+1.f, y+1.f, z+0.f, 1.f, 0.f, Orientation::Top,
                x+1.f, y+1.f, z+1.f, 1.f, 1.f, Orientation::Top,

                x+0.f, y+1.f, z+0.f, 0.f, 0.f, Orientation::Top,
                x+1.f, y+1.f, z+1.f, 1.f, 1.f, Orientation::Top,
                x+0.f, y+1.f, z+1.f, 0.f, 1.f, Orientation::Top,
            });

        // left
        i = XYZtoIndex(x-1, y, z);
        if (i == -1 || chunk.blocks[i] == BlockType::Air)
            v.insert(v.end(), {
                x+0.f, y+0.f, z+0.f, 0.f, 0.f, Orientation::Left,
                x+0.f, y+1.f, z+1.f, 1.f, 1.f, Orientation::Left,
                x+0.f, y+0.f, z+1.f, 0.f, 1.f, Orientation::Left,

                x+0.f, y+0.f, z+0.f, 0.f, 0.f, Orientation::Left,
                x+0.f, y+1.f, z+0.f, 1.f, 0.f, Orientation::Left,
                x+0.f, y+1.f, z+1.f, 1.f, 1.f, Orientation::Left,
            });

        // right
        i = XYZtoIndex(x+1, y, z);
        if (i == -1 || chunk.blocks[i] == BlockType::Air)
            v.insert(v.end(), {
                x+1.f, y+0.f, z+0.f, 0.f, 0.f, Orientation::Right,
                x+1.f, y+0.f, z+1.f, 0.f, 1.f, Orientation::Right,
                x+1.f, y+1.f, z+1.f, 1.f, 1.f, Orientation::Right,

                x+1.f, y+0.f, z+0.f, 0.f, 0.f, Orientation::Right,
                x+1.f, y+1.f, z+1.f, 1.f, 1.f, Orientation::Right,
                x+1.f, y+1.f, z+0.f, 1.f, 0.f, Orientation::Right,
            });
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
}

Chunk_t generateChunk(glm::ivec2 pos)
{
    Chunk_t chunk;

    chunk.pos = pos;

    srand(time(NULL));

    for (int z = 0 ; z < 16 ; ++z) {
    for (int x = 0 ; x < 16 ; ++x) {
        int height = 1 + rand() % 8;
    for (int y = 0 ; y < 16 ; ++y) {
        int index = z * 16*16 + y * 16 + x;

        // chunk.blocks[index] = BlockType::Grass;
        chunk.blocks[index] = y < height ? BlockType::Grass : BlockType::Air;
        // chunk.blocks[index] = rand() % 3 == 0 ? BlockType::Grass : BlockType::Air;
    }
    }
    }

    computeChunckVAO(chunk);

    return chunk;
}

class GameView: public View {
    public:
        GameView(Context& ctx): View(ctx)
        {
            int width, height;
            glfwGetWindowSize(ctx.window, &width, &height);

            camera = new OrbitCamera(
                glm::vec3(0.0f), 0.0f, 0.0f, 10.0f,
                60.0f, (float)width / (float)height, 0.01f, 1000.0f
            );

            cube_shader = new Program("./assets/shaders/cube.vs", "./assets/shaders/cube.fs");

            Chunk c = generateChunk({0, 0});
            chunks[c.pos] = c;
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

        void onMouseMotion(int x, int y, int dx, int dy)
        {
        }

        void onMouseDrag(int x, int y, int dx, int dy)
        {
            if (ImGui::GetIO().WantCaptureMouse) return;

            camera->setYaw( camera->getYaw() - (dx * 0.005f) );
            camera->setPitch( camera->getPitch() + (dy * 0.005f) );
        }

        void onMousePress(int x, int y, int button)
        {
        }

        void onMouseRelease(int x, int y, int button)
        {
        }

        void onMouseScroll(int scroll_x, int scroll_y)
        {
            float scale = 1.0f;
            camera->setDistance( camera->getDistance() - (scroll_y * scale) );
        }

        void onResize(int width, int height)
        {
            glViewport(0, 0, width, height);
        }

    private:
        OrbitCamera* camera;
        Program* cube_shader;

        GLuint VAO;

        // std::map<std::pair<int, int>, Chunk_t> chunks;

        std::unordered_map<glm::ivec2, Chunk_t> chunks;
};
