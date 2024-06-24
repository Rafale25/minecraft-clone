#pragma once

#include "view.hpp"
#include "camera.hpp"
#include "fps_camera.hpp"
#include "program.h"

#include "texture_manager.hpp"
#include "chunk.hpp"
#include "client.hpp"
#include "entity.hpp"
#include "world.hpp"

#include "texture.hpp"
#include "framebuffer.hpp"
#include "geometry.hpp"
#include "shadow_map.hpp"

#include "imgui.h"
#include <algorithm>
#include <string>

#include "string_helpers.hpp"

class GameView: public View {
    public:
        GameView(Context& ctx): View(ctx)
        {
            glfwSetInputMode(ctx.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

            texture_manager.loadAllTextures();

            cube_shader.use();
            cube_shader.setInt("shadowMap", 0);


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

            const int MAX_NEW_CHUNKS_PER_FRAME = 3;
            int i = 0;

            const glm::vec3 camPos = camera.getPosition();
            std::sort(client.new_chunks.begin(), client.new_chunks.end(),
                [this, camPos](const Chunk* l, const Chunk* r)
                {
                    return glm::distance2(camPos, glm::vec3(l->pos*16)) > glm::distance2(camPos, glm::vec3(r->pos*16));
                });

            // TODO: make a third thread to compute VBO and then do OpenGL calls on main thread

            while (client.new_chunks.size() > 0 && i++ < MAX_NEW_CHUNKS_PER_FRAME)
            {
                Chunk* c = client.new_chunks.back();
                client.new_chunks.pop_back();

                world.set_chunk(c);
                c->computeChunckVAO(world, texture_manager);

                // recompute neighbours chunks VAO //
                const glm::ivec3 offsets[] = { {-1, 0, 0}, {1, 0, 0}, {0, -1, 0}, {0, 1, 0}, {0, 0, -1}, {0, 0, 1} };

                for (const glm::ivec3 &offset: offsets)
                {
                    glm::ivec3 cpos = c->pos + offset;

                    Chunk* nc = world.get_chunk(cpos);
                    if (nc != nullptr) {
                        nc->computeChunckVAO(world, texture_manager);
                    }
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

            shadowmap.setSunDir(sunDir);

            shadowmap.begin(camera, cube_shadowmapping_shader);
            render_world(cube_shadowmapping_shader);
            shadowmap.end();

            cube_shader.use();
            cube_shader.setMat4("u_lightSpaceMatrix", shadowmap._lightSpaceMatrix);
            cube_shader.setVec3("u_sun_direction", sunDir);
            cube_shader.setFloat("u_shadow_bias", shadowmap._shadow_bias);

            glBindTextureUnit(0, shadowmap._depthTexture->_texture);
            render_world(cube_shader);

            mesh_shader.use();
            mesh_shader.setMat4("u_projectionMatrix", camera.getProjection());
            mesh_shader.setMat4("u_viewMatrix", camera.getView());

            for (auto& entity : world.entities)
            {
                mesh_shader.setMat4("u_modelMatrix", entity.smooth_transform.getMatrix());
                entity.draw();
            }

            if (_show_debug_gui) gui(dt);
        }

        void render_world(Program &shader)
        {
            shader.use();
            shader.setMat4("u_projectionMatrix", camera.getProjection());
            shader.setMat4("u_viewMatrix", camera.getView());
            shader.setVec3("u_view_position", camera.getPosition());

            for (const auto& [key, chunk] : world.chunks)
            {
                shader.setVec3("u_chunkPos", chunk->pos * 16);
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, chunk->mesh.ssbo_texture_handles);
                glBindVertexArray(chunk->mesh.VAO);
                glDrawElements(GL_TRIANGLES, chunk->mesh.indices_count, GL_UNSIGNED_INT, 0);
            }
        }

        void gui(float dt)
        {

            ctx.imguiNewFrame();
            // ImGui::ShowDemoWindow();

            ImGui::Begin("Shadow map");
            ImGui::Image((ImTextureID)shadowmap._depthTexture->_texture, ImVec2(ctx.width/3, ctx.height/3), ImVec2(0, 1), ImVec2(1, 0));
            ImGui::End();

            ImGui::Begin("Debug");

            ImGui::Text("%.4f secs", dt);
            ImGui::Text("%.2f fps", 1.0f / dt);

            glm::vec3 camera_pos = camera.getPosition();
            ImGui::Text("position: %.2f, %.2f, %.2f", camera_pos.x, camera_pos.y, camera_pos.z);
            ImGui::Text("forward: %.2f, %.2f, %.2f", camera.forward.x, camera.forward.y, camera.forward.z);
            ImGui::Text("block in hand: %d", (int)blockInHand);

            if (ImGui::TreeNode(SC("Entities: " << world.entities.size()))) {
                for (auto& entity : world.entities) {
                    ImGui::PushID(entity.id);
                    ImGui::Text("id:%d: x:%.2f y:%.2f z:%.2f", entity.id, entity.transform.position.x, entity.transform.position.y, entity.transform.position.z);
                    ImGui::SameLine();
                    if (ImGui::Button("teleport")) {
                        set_player_position(entity.transform.position);
                    }
                    ImGui::PopID();
                }
                ImGui::TreePop();
            }


            ImGui::SliderFloat("Bulk Edit Radius: ", &bulkEditRadius, 1.0f, 32.0f, "%.2f");
            ImGui::Checkbox("Wireframe", &_wireframe);
            if (ImGui::Checkbox("VSync", &_vsync)) {
                ctx.setVsync(_vsync);
            }

            ImGui::DragFloat3("Sun direction: ", &sunDir.x, 0.01f, -M_PI*2, M_PI*2, "%.2f");
            ImGui::SliderFloat("Shadow Bias: ", &shadowmap._shadow_bias, 0.000001f, 0.1f, "%.6f");
            ImGui::SliderFloat("Shadow Distance: ", &shadowmap._max_shadow_distance, 0.3f, 500.0f, "%.2f");

            ImGui::End();
            ctx.imguiRender();
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

        void set_player_position(glm::vec3 p) {
            camera.setPosition(p);
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

            if (key == GLFW_KEY_P) {
                _show_debug_gui = !_show_debug_gui;
            }
        }

        void onMousePress(int x, int y, int button) {
            if (_show_debug_gui && ImGui::GetIO().WantCaptureMouse) return;

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
            if (_show_debug_gui && ImGui::GetIO().WantCaptureMouse) return;
        }

        void onMouseScroll(int scroll_x, int scroll_y)
        {
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
            camera.aspect_ratio = (float)width / (float)height;
        }

    private:
        Program cube_shader{"./assets/shaders/cube.vs", "./assets/shaders/cube.fs"};
        Program cube_shadowmapping_shader{"./assets/shaders/cube_shadowmap.vs", "./assets/shaders/cube_shadowmap.fs"};
        Program mesh_shader{"./assets/shaders/mesh.vs", "./assets/shaders/mesh.fs"};
        Program debugquad_shader{"./assets/shaders/debug_quad.vs", "./assets/shaders/debug_quad_depth.fs"};

        Shadowmap shadowmap{ctx, 4096, 4096};
        glm::vec3 sunDir = glm::normalize(glm::vec3(20.0f, 50.0f, 20.0f));
        // Mesh _debugQuad = Geometry::quad_2d();

        TextureManager texture_manager;

        World world;
        Client client{world, texture_manager, "51.77.194.124"};

        float network_timer = 1.0f;


        bool _cursorEnable = false;
        bool _show_debug_gui = false;
        bool _wireframe = false;
        bool _vsync = true;

        // Player
        FPSCamera camera = {
            glm::vec3(10.0f, 20.0, 12.0f), 0.0f, 0.0f,
            60.0f, (float)ctx.width / (float)ctx.height, 0.1f, 1000.0f
        };

        BlockType blockInHand = BlockType::Grass;
        float bulkEditRadius = 4.0f;

        BlockType raycastBlocktype;
        glm::vec3 raycastNormal;
        glm::ivec3 raycastWorldPos;
        // -- //
};

/*
camera = new OrbitCamera(
    glm::vec3(0.0f), M_PI/4, M_PI/4, 50.0f,
    60.0f, (float)width / (float)height, 0.1f, 1000.0f
);
*/
