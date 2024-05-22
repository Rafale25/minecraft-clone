#pragma once

#include <unordered_map>
#include <map>
#include <math.h>

#include "glm/gtx/hash.hpp"
#include <glm/gtc/type_ptr.hpp>

#include "FastNoiseLite.h"

#include "view.hpp"
#include "camera.hpp"
#include "program.h"

#include "texture_manager.hpp"
#include "chunk.hpp"
#include "enums.hpp"

Chunk generateChunk(glm::ivec3 pos, FastNoiseLite &noise, TextureManager &texture_manager)
{
    Chunk chunk;

    chunk.pos = pos;

    glm::vec3 chunkPosWorld = pos * 16;

    for (int z = 0 ; z < 16 ; ++z) {
    for (int x = 0 ; x < 16 ; ++x) {
        int height = 1.0f + (noise.GetNoise(chunkPosWorld.x + x, chunkPosWorld.z + z) * 0.5f + 0.5f) * 15.0f;

        for (int y = 0 ; y < 16 ; ++y) {
            int index = z * 16*16 + y * 16 + x;

            float world_y = chunkPosWorld.y + y;

            if (world_y == height)
                chunk.blocks[index] = BlockType::Grass;
            else if (world_y < height)
                chunk.blocks[index] = BlockType::Dirt;
            else
                chunk.blocks[index] = BlockType::Air;

            // chunk.blocks[index] = BlockType::Grass;
            // chunk.blocks[index] = chunkPosWorld.y + y < height ? BlockType::Grass : BlockType::Air;
            // chunk.blocks[index] = rand() % 3 == 0 ? BlockType::Grass : BlockType::Air;
        }
    }
    }

    chunk.computeChunckVAO(texture_manager);

    return chunk;
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

            texture_manager.loadAllTextures();

            cube_shader = new Program("./assets/shaders/cube.vs", "./assets/shaders/cube.fs");

            int SIZE_X = 4;
            int SIZE_Y = 1;
            int SIZE_Z = 4;
            for (int x = -SIZE_X ; x < SIZE_X ; ++x) {
            for (int y = -SIZE_Y ; y < SIZE_Y ; ++y) {
            for (int z = -SIZE_Z ; z < SIZE_Z ; ++z) {
                Chunk c = generateChunk({x, y, z}, noise, texture_manager);
                chunks[c.pos] = c;
            }
            }
            }

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

        std::unordered_map<glm::ivec3, Chunk> chunks;

        FastNoiseLite noise;
        TextureManager texture_manager;
};
