#pragma once

#include "view.hpp"
#include "camera.hpp"
#include "program.h"

#include "texture_manager.hpp"
#include "chunk.hpp"
#include "client.hpp"
#include "entity.hpp"
#include "world.hpp"

#include "imgui.h"

class GameView: public View {
    public:
        GameView(Context& ctx): View(ctx)
        {
            glfwSetInputMode(ctx.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

            int width, height;
            glfwGetWindowSize(ctx.window, &width, &height);

            // camera = new OrbitCamera(
            //     glm::vec3(0.0f), M_PI/4, M_PI/4, 50.0f,
            //     60.0f, (float)width / (float)height, 0.1f, 1000.0f
            // );

            camera = FPSCamera{
                glm::vec3(10.0f, 20.0, 12.0f), 0.0f, 0.0f,
                60.0f, (float)width / (float)height, 0.01f, 1000.0f
            };

            texture_manager.loadAllTextures();

            client.Start();
        }

        void onUpdate(double time_since_start, float dt)
        {
            float dx = ctx.keyState[GLFW_KEY_A] - ctx.keyState[GLFW_KEY_D];
            float dy = ctx.keyState[GLFW_KEY_LEFT_CONTROL] - ctx.keyState[GLFW_KEY_SPACE];
            float dz = ctx.keyState[GLFW_KEY_W] - ctx.keyState[GLFW_KEY_S];

            camera.setSpeed(
                ctx.keyState[GLFW_KEY_LEFT_SHIFT] == GLFW_PRESS ? 30.0f : 10.0f
            );

            camera.move(glm::vec3(dx, dy, dz));
            camera.update(dt);

            consume_task_queue();
            consume_new_chunks();

            world.update_entities();

            auto [blocktype, world_pos, normal] = world.BlockRaycast(camera.getPosition(), camera.forward, 16);
            raycastBlocktype = blocktype;
            raycastWorldPos = world_pos;
            raycastNormal = normal;

            network_timer -= dt;
            if (network_timer <= 0.0f) {
                network_timer = 1.0f / 20.0f;
                networkUpdate();
            }
        }

        void consume_new_chunks()
        {
            client.new_chunks_mutex.lock();

            const int MAX_NEW_CHUNKS_PER_FRAME = 16;
            int i = 0;

            // TODO: make a third thread to compute VBO and then do OpenGL calls on main thread

            while (client.new_chunks.size() > 0 && i++ < MAX_NEW_CHUNKS_PER_FRAME)
            {
                Chunk c = client.new_chunks.back();
                client.new_chunks.pop_back();

                world.chunks[c.pos] = c;
                world.chunks[c.pos].computeChunckVAO(world, texture_manager);

                // recompute neighbours chunks VAO //
                const glm::ivec3 offsets[] = { {-1, 0, 0}, {1, 0, 0}, {0, -1, 0}, {0, 1, 0}, {0, 0, -1}, {0, 0, 1} };

                for (const glm::ivec3 &offset: offsets)
                {
                    glm::ivec3 cpos = c.pos + offset;

                    // if (world.chunks.count(cpos) > 0)
                    if (world.chunks.find(cpos) != world.chunks.end())
                        world.chunks[cpos].computeChunckVAO(world, texture_manager);
                }

            }
            client.new_chunks_mutex.unlock();
        }

        void consume_task_queue()
        {
            client.task_queue_mutex.lock();
            for (auto &task: client.task_queue) {
                task();
            }
            client.task_queue.clear();
            client.task_queue_mutex.unlock();
        }

        void networkUpdate()
        {
            if (client.client_id == -1) return;

            glm::vec3 pos = camera.getPosition();
            float yaw = camera.getYaw();
            float pitch = camera.getPitch();

            client.sendUpdateEntityPacket(client.client_id, pos, yaw, pitch);
        }

        void onDraw(double time_since_start, float dt)
        {
            glEnable(GL_MULTISAMPLE); // enabled by default

            glPolygonMode(GL_FRONT_AND_BACK, _wireframe ? GL_LINE : GL_FILL);

            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);

            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
            glFrontFace(GL_CW);

            glClearColor(135.0f/255.0f, 206.0f/255.0f, 250.0f/255.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            cube_shader.use();
            cube_shader.setMat4("u_projectionMatrix", camera.getProjection());
            cube_shader.setMat4("u_viewMatrix", camera.getView());
            cube_shader.setVec3("u_view_position", camera.getPosition());

            for (const auto& [key, chunk] : world.chunks)
            {
                cube_shader.setVec3("u_chunkPos", chunk.pos * 16);
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, chunk.ssbo_texture_handles);
                glBindVertexArray(chunk.VAO);
                glDrawArrays(GL_TRIANGLES, 0, chunk.vertex_count);
            }

            mesh_shader.use();
            mesh_shader.setMat4("u_projectionMatrix", camera.getProjection());
            mesh_shader.setMat4("u_viewMatrix", camera.getView());

            for (auto& entity : world.entities)
            {
                mesh_shader.setMat4("u_modelMatrix", entity.smooth_transform.getMatrix());
                entity.draw();
            }

            gui(dt);
        }

        void gui(float dt)
        {
            ctx.imguiNewFrame();
            ImGui::Begin("Debug");

            ImGui::Text("%.4f secs", dt);
            ImGui::Text("%.2f fps", 1.0f / dt);

            glm::vec3 camera_pos = camera.getPosition();
            ImGui::Text("center: %.2f, %.2f, %.2f", camera_pos.x, camera_pos.y, camera_pos.z);
            ImGui::Text("forward: %.2f, %.2f, %.2f", camera.forward.x, camera.forward.y, camera.forward.z);
            ImGui::Text("block in hand: %d", (int)blockInHand);
            ImGui::Text("yaw: %.2f", camera.getYaw());
            ImGui::Text("pitch: %.2f", camera.getPitch());

            ImGui::SliderFloat("Bulk Edit Radius: ", &bulkEditRadius, 1.0f, 32.0f, "%.2f");
            ImGui::Checkbox("Wireframe", &_wireframe);
            if (ImGui::Checkbox("VSync", &_vsync)) {
                ctx.setVsync(_vsync);
            }

            ImGui::End();
            ctx.imguiRender();
        }

        void onKeyPress(int key)
        {
            if (key == GLFW_KEY_C) {
                _cursorEnable = !_cursorEnable;

                if (_cursorEnable)
                    glfwSetInputMode(ctx.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                else
                    glfwSetInputMode(ctx.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            }

            if (key == GLFW_KEY_F11) {
                // GLFWmonitor* monitor = glfwGetWindowMonitor(ctx.window);
                // GLFWmonitor* monitor = glfwGetPrimaryMonitor();
                // const GLFWvidmode* mode = glfwGetVideoMode(monitor);
                // glfwSetWindowMonitor(ctx.window, monitor, 0, 0, mode->width, mode->height, 0);
            }
        }

        void placeSphere(glm::ivec3 pos, float radius, BlockType blocktype)
        {
            std::vector<glm::ivec3> positions;

            int iradius = int(radius);
            for (int x = -iradius ; x <= iradius ; ++x) {
            for (int y = -iradius ; y <= iradius ; ++y) {
            for (int z = -iradius ; z <= iradius ; ++z) {
                glm::ivec3 wpos = pos + glm::ivec3{x, y, z};
                if (glm::distance2(glm::vec3(pos), glm::vec3(wpos)) > radius*radius) continue;
                positions.push_back(wpos);
            }
            }
            }
            client.sendBlockBulkEditPacket(positions, blocktype);
        }

        void onMousePress(int x, int y, int button) {
            if (ImGui::GetIO().WantCaptureMouse) return;

            if (button == GLFW_MOUSE_BUTTON_LEFT) {
                if (ctx.keyState[GLFW_KEY_LEFT_ALT])
                    placeSphere(raycastWorldPos, bulkEditRadius, BlockType::Air);
                else
                    client.sendBreakBlockPacket(raycastWorldPos);
            } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
                if (ctx.keyState[GLFW_KEY_LEFT_ALT])
                    placeSphere(raycastWorldPos, bulkEditRadius, blockInHand);
                else
                    client.sendPlaceBlockPacket(raycastWorldPos + glm::ivec3(raycastNormal), blockInHand);
            }
        }

        void onMouseDrag(int x, int y, int dx, int dy)
        {
            if (ImGui::GetIO().WantCaptureMouse) return;

            // camera.setYaw( camera.getYaw() - (dx * 0.005f) );
            // camera.setPitch( camera.getPitch() + (dy * 0.005f) );
        }

        void onMouseScroll(int scroll_x, int scroll_y)
        {
            // float scale = 1.0f;
            // if (ctx.keyState[GLFW_KEY_LEFT_SHIFT] == GLFW_PRESS)
            //     scale = 8.0f;
            // camera.setDistance( camera.getDistance() - (scroll_y * scale) );

            int block = ((int)blockInHand + scroll_y) % ((int)BlockType::LAST-1);
            if (block < 1)
                block += (int)BlockType::LAST-1;
            blockInHand = (BlockType)block;
        }

        void onMouseMotion(int x, int y, int dx, int dy)
        {
            if (!_cursorEnable)
                camera.onMouseMotion(x, y, dx, dy);
        }

        void onResize(int width, int height)
        {
            glViewport(0, 0, width, height);
        }

    private:
        Program cube_shader{"./assets/shaders/cube.vs", "./assets/shaders/cube.fs"};
        Program mesh_shader{"./assets/shaders/mesh.vs", "./assets/shaders/mesh.fs"};

        TextureManager texture_manager;

        World world;
        Client client{world, texture_manager, "51.77.194.124"};

        float network_timer = 1.0f;
        bool _cursorEnable = false;

        bool _wireframe = false;
        bool _vsync = true;

        // Player
        FPSCamera camera;

        BlockType blockInHand = BlockType::Grass;
        float bulkEditRadius = 4.0f;

        BlockType raycastBlocktype;
        glm::vec3 raycastNormal;
        glm::ivec3 raycastWorldPos;
        // -- //
};

/*
#if 0
    FastNoiseLite noise;
    noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);

    int SIZE_X = 8;
    int SIZE_Y = 1;
    int SIZE_Z = 8;
    for (int x = -SIZE_X ; x < SIZE_X ; ++x) {
    for (int y = -SIZE_Y ; y < SIZE_Y ; ++y) {
    for (int z = -SIZE_Z ; z < SIZE_Z ; ++z) {
        Chunk c = generateChunkWithNoise({x, y, z}, noise, world, texture_manager);
        world.chunks[c.pos] = c;
    }
    }
    }
#endif


#include "FastNoiseLite.h"

Chunk generateChunkWithNoise(glm::ivec3 pos, FastNoiseLite &noise, World &world, TextureManager &texture_manager)
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

    chunk.computeChunckVAO(world, texture_manager);

    return chunk;
}


*/
