#pragma once

#include <unordered_map>
#include <map>

#include <glm/gtc/type_ptr.hpp>

#include "View.hpp"
#include "Camera.hpp"
#include "Program.h"


enum BlockType : uint8_t {
    Air = 0,
    Grass = 1,
};

typedef struct Chunk
{
    int x, y;
    GLuint VAO;
    uint vertices_count;
    BlockType blocks[4096]; // 16 x 16 x 16
} Chunk_t;

void computeChunckVAO(Chunk_t &chunk)
{
    glCreateVertexArrays(1, &chunk.VAO);

    GLuint VBO;
    glCreateBuffers(1, &VBO);

    std::vector<glm::vec3> v;
    // std::vector<float> vertices;

    for (int z = 0 ; z < 16 ; ++z) {
    for (int y = 0 ; y < 16 ; ++y) {
    for (int x = 0 ; x < 16 ; ++x) {

        glm::vec3 pos{x, y, z};

        // front
        v.push_back(pos + glm::vec3(0, 0, 0)); // xyz
        // v.push_back(); // uv
        // v.insert(v.end(), { x+0.0f, y+0.0f, z+0.0f, 0.0f, 0.0f });

        v.push_back(pos + glm::vec3(1, 0, 0));
        v.push_back(pos + glm::vec3(1, 1, 0));

        // v.insert(v.end(), {0,1,2,3,4,5,6,7,8,9});

        v.push_back(pos + glm::vec3(0, 0, 0));
        v.push_back(pos + glm::vec3(1, 1, 0));
        v.push_back(pos + glm::vec3(0, 1, 0));

        // back
        v.push_back(pos + glm::vec3(0, 0, 1));
        v.push_back(pos + glm::vec3(1, 0, 1));
        v.push_back(pos + glm::vec3(1, 1, 1));

        v.push_back(pos + glm::vec3(0, 0, 1));
        v.push_back(pos + glm::vec3(1, 1, 1));
        v.push_back(pos + glm::vec3(0, 1, 1));

        // down
        v.push_back(pos + glm::vec3(0, 0, 0));
        v.push_back(pos + glm::vec3(1, 0, 0));
        v.push_back(pos + glm::vec3(1, 0, 1));

        v.push_back(pos + glm::vec3(0, 0, 0));
        v.push_back(pos + glm::vec3(1, 0, 1));
        v.push_back(pos + glm::vec3(0, 0, 1));

        // top
        v.push_back(pos + glm::vec3(0, 1, 0));
        v.push_back(pos + glm::vec3(1, 1, 0));
        v.push_back(pos + glm::vec3(1, 1, 1));

        v.push_back(pos + glm::vec3(0, 1, 0));
        v.push_back(pos + glm::vec3(1, 1, 1));
        v.push_back(pos + glm::vec3(0, 1, 1));

        // left
        v.push_back(pos + glm::vec3(0, 0, 0));
        v.push_back(pos + glm::vec3(0, 0, 1));
        v.push_back(pos + glm::vec3(0, 1, 1));

        v.push_back(pos + glm::vec3(0, 0, 0));
        v.push_back(pos + glm::vec3(0, 1, 1));
        v.push_back(pos + glm::vec3(0, 1, 0));

        // right
        v.push_back(pos + glm::vec3(1, 0, 0));
        v.push_back(pos + glm::vec3(1, 0, 1));
        v.push_back(pos + glm::vec3(1, 1, 1));

        v.push_back(pos + glm::vec3(1, 0, 0));
        v.push_back(pos + glm::vec3(1, 1, 1));
        v.push_back(pos + glm::vec3(1, 1, 0));
    }
    }
    }

    chunk.vertices_count = v.size() * 3;

    glNamedBufferData(VBO, chunk.vertices_count  * sizeof(float), &v[0], GL_STATIC_DRAW);

    glEnableVertexArrayAttrib(chunk.VAO, 0);
    glVertexArrayAttribBinding(chunk.VAO, 0, 0);
    glVertexArrayAttribFormat(chunk.VAO, 0, 3, GL_FLOAT, GL_FALSE, 0);

    glVertexArrayVertexBuffer(chunk.VAO, 0, VBO, 0, 3*sizeof(GL_FLOAT));
}

Chunk_t generateChunk(int x, int y)
{
    Chunk_t chunk;

    chunk.x = x;
    chunk.y = y;

    for (int z = 0 ; z < 16 ; ++z) {
    for (int y = 0 ; y < 16 ; ++y) {
    for (int x = 0 ; x < 16 ; ++x) {
        int index = z * 16*16 + y * 16 + x;
        chunk.blocks[index] = BlockType::Grass;
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

            Chunk c = generateChunk(0, 0);
            chunks[{c.x, c.y}] = c;
        }

        void onUpdate(float time_since_start, float dt)
        {
        }

        void onDraw(float time_since_start, float dt)
        {
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);

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

        std::map<std::pair<int, int>, Chunk_t> chunks;
};
