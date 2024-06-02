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
#include <algorithm>

std::vector<glm::vec4> getFrustumCornersWorldSpace(const glm::mat4& proj, const glm::mat4& view)
{
    const auto inv = glm::inverse(proj * view);

    std::vector<glm::vec4> frustumCorners;
    for (unsigned int x = 0; x < 2; ++x) {
        for (unsigned int y = 0; y < 2; ++y) {
            for (unsigned int z = 0; z < 2; ++z) {
                const glm::vec4 pt =
                    inv * glm::vec4(
                        2.0f * x - 1.0f,
                        2.0f * y - 1.0f,
                        2.0f * z - 1.0f,
                        1.0f);
                frustumCorners.push_back(pt / pt.w);
            }
        }
    }

    return frustumCorners;
}

glm::mat4 getLighViewMatrix(std::vector<glm::vec4> cameraFrustumCorners, glm::vec3 lightDir)
{
    glm::vec3 center = glm::vec3(0, 0, 0);
    for (const auto& v : cameraFrustumCorners) {
        center += glm::vec3(v);
    }
    center /= cameraFrustumCorners.size();

    return glm::lookAt(
        center + lightDir,
        center,
        glm::vec3(0.0f, 1.0f, 0.0f)
    );
}

struct FrustumBounds {
    float minX, maxX;
    float minY, maxY;
    float minZ, maxZ;
};

FrustumBounds computeFrustumBounds(const glm::mat4& lightView, const std::vector<glm::vec4>& corners)
{
    FrustumBounds b;

    b.minX = std::numeric_limits<float>::max();
    b.maxX = std::numeric_limits<float>::lowest();
    b.minY = std::numeric_limits<float>::max();
    b.maxY = std::numeric_limits<float>::lowest();
    b.minZ = std::numeric_limits<float>::max();
    b.maxZ = std::numeric_limits<float>::lowest();

    for (const auto& v : corners)
    {
        const glm::vec4 trf = lightView * v;
        b.minX = std::min(b.minX, trf.x);
        b.maxX = std::max(b.maxX, trf.x);
        b.minY = std::min(b.minY, trf.y);
        b.maxY = std::max(b.maxY, trf.y);
        b.minZ = std::min(b.minZ, trf.z);
        b.maxZ = std::max(b.maxZ, trf.z);
    }

    return b;
}

glm::mat4 getLightProjectionMatrix(const glm::mat4& lightView, FrustumBounds& b)
{
    // Tune this parameter according to the scene
    const float zMult = 5.0f;
    if (b.minZ < 0)
        b.minZ *= zMult;
    else
        b.minZ /= zMult;

    if (b.maxZ < 0)
        b.maxZ /= zMult;
    else
        b.maxZ *= zMult;

    return glm::ortho(b.minX, b.maxX, b.minY, b.maxY, b.minZ, b.maxZ);
}

class GameView: public View {
    public:
        GameView(Context& ctx): View(ctx)
        {
            glfwSetInputMode(ctx.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            glfwGetWindowSize(ctx.window, &_width, &_height);
            onResize(_width, _height);

            texture_manager.loadAllTextures();

            // -- Depth FBO
            glGenFramebuffers(1, &depthMapFBO);
            glGenTextures(1, &_depthMap);
            glBindTexture(GL_TEXTURE_2D, _depthMap);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
            glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

            glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _depthMap, 0);
            glDrawBuffer(GL_NONE);
            glReadBuffer(GL_NONE);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            cube_shader.use();
            cube_shader.setInt("shadowMap", 0);
            // --

            // renderQuad() renders a 1x1 XY quad in NDC
            // -----------------------------------------
            float quadVertices[] = {
                // positions        // texture Coords
                -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
                -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
                1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
            };
            // setup plane VAO
            glGenVertexArrays(1, &_quadVAO);
            glGenBuffers(1, &_quadVBO);
            glBindVertexArray(_quadVAO);
            glBindBuffer(GL_ARRAY_BUFFER, _quadVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
            // -----------------------------------------

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

            // 1. render depth of scene to texture (from light's perspective)
            // --------------------------------------------------------------
            // glm::mat4 cameraCustomProj = glm::perspective(camera.fov, camera.aspect_ratio, 1.0f, 20.0f);
            // auto corners = getFrustumCornersWorldSpace(cameraCustomProj, camera.getView());
            auto corners = getFrustumCornersWorldSpace(camera.getProjection(), camera.getView());

            glm::mat4 lightViewMatrix = getLighViewMatrix(corners, sunDir);
            FrustumBounds bounds = computeFrustumBounds(lightViewMatrix, corners);
            glm::mat4 lightProjectionMatrix = getLightProjectionMatrix(lightViewMatrix, bounds);
            glm::mat4 lightSpaceMatrix = lightProjectionMatrix * lightViewMatrix;

            cube_shadowmapping_shader.use();
            cube_shadowmapping_shader.setMat4("u_lightSpaceMatrix", lightSpaceMatrix);
            glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
            glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
                glClear(GL_DEPTH_BUFFER_BIT);

                // glCullFace(GL_FRONT); // fix shadow acne
                render_scene(cube_shadowmapping_shader);
                // glCullFace(GL_BACK); // fix shadow acne

            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            // reset viewport
            glViewport(0, 0, _width, _height);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            cube_shader.use();
            cube_shader.setMat4("u_lightSpaceMatrix", lightSpaceMatrix);
            cube_shader.setVec3("u_sun_direction", sunDir);
            cube_shader.setFloat("near_plane", bounds.minZ);
            cube_shader.setFloat("far_plane", bounds.maxZ);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, _depthMap);
            render_scene(cube_shader);

            mesh_shader.use();
            mesh_shader.setMat4("u_projectionMatrix", camera.getProjection());
            mesh_shader.setMat4("u_viewMatrix", camera.getView());

            for (auto& entity : world.entities)
            {
                mesh_shader.setMat4("u_modelMatrix", entity.smooth_transform.getMatrix());
                entity.draw();
            }


            glViewport(_width-_width/3, _height-_height/3, _width/3, _height/3);

            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
            debugquad_shader.use();
            debugquad_shader.setInt("depthMap", 0);
            debugquad_shader.setFloat("near_plane", bounds.minZ);
            debugquad_shader.setFloat("far_plane", bounds.maxZ);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, _depthMap);
            glBindVertexArray(_quadVAO);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);


            gui(dt);
        }

        void render_scene(Program &shader)
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
                glDrawArrays(GL_TRIANGLES, 0, chunk->mesh.vertex_count);
            }
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
            _width = width;
            _height = height;
            camera.aspect_ratio = (float)width / (float)height;
        }

    private:
        Program cube_shader{"./assets/shaders/cube.vs", "./assets/shaders/cube.fs"};
        Program cube_shadowmapping_shader{"./assets/shaders/cube_shadowmap.vs", "./assets/shaders/cube_shadowmap.fs"};
        Program mesh_shader{"./assets/shaders/mesh.vs", "./assets/shaders/mesh.fs"};
        Program debugquad_shader{"./assets/shaders/debug_quad.vs", "./assets/shaders/debug_quad_depth.fs"};

        // Shadow map
        const uint32_t SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;
        GLuint depthMapFBO;
        GLuint _depthMap;

        GLuint _quadVAO = 0;
        GLuint _quadVBO;

        glm::vec3 sunDir = glm::normalize(glm::vec3(20.0f, 50.0f, 20.0f));
        // --

        TextureManager texture_manager;

        World world;
        Client client{world, texture_manager, "51.77.194.124"};

        float network_timer = 1.0f;
        bool _cursorEnable = false;

        bool _wireframe = false;
        bool _vsync = true;

        // Player
        FPSCamera camera = {
            glm::vec3(10.0f, 20.0, 12.0f), 0.0f, 0.0f,
            60.0f, (float)_width / (float)_height, 0.1f, 20.0f
        };

        BlockType blockInHand = BlockType::Grass;
        float bulkEditRadius = 4.0f;

        BlockType raycastBlocktype;
        glm::vec3 raycastNormal;
        glm::ivec3 raycastWorldPos;
        // -- //

        int _width, _height;
};

/*
camera = new OrbitCamera(
    glm::vec3(0.0f), M_PI/4, M_PI/4, 50.0f,
    60.0f, (float)width / (float)height, 0.1f, 1000.0f
);

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
