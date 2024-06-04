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

class Brush {
    public:
        enum Shape {
            SPHERE,
            CUBE,
            RAY
        };

        enum Action {
            PLACE = 0,
            PAINT = 1,
            PAINT_RANDOMIZE = 2,
            RANDOMIZE = 3,
        };

    public:
        Brush(World& world, Client& client): _world(world), _client(client) {};

        void brush(Shape shape, Action action, BlockType blocktype, glm::vec3 center, int size) {
            std::vector<glm::ivec3> block_pos;
            std::vector<BlockType> block_types;

            switch (shape) {
                case SPHERE:
                    block_pos = getBlocksSphere(center, size);
                    break;
                case CUBE:
                    block_pos = getBlocksCube(center, size);
                    break;
                case RAY:
                    break;
            }

            switch (action) {
                case PLACE:
                    block_types = std::vector<BlockType>(block_pos.size(), blocktype);
                    break;
                case PAINT: {
                    std::vector<glm::ivec3> filtered_block_pos;
                    std::copy_if (block_pos.begin(), block_pos.end(), std::back_inserter(filtered_block_pos), [this](glm::ivec3 pos) {return _world.get_block(pos) != BlockType::Air ;} );
                    block_pos = filtered_block_pos;
                    block_types = std::vector<BlockType>(block_pos.size(), blocktype);
                    break;
                }
                case PAINT_RANDOMIZE: {
                    std::vector<glm::ivec3> filtered_block_pos;
                    std::copy_if (block_pos.begin(), block_pos.end(), std::back_inserter(filtered_block_pos), [this](glm::ivec3 pos) {return _world.get_block(pos) != BlockType::Air ;} );
                    block_pos = filtered_block_pos;

                    block_types = std::vector<BlockType>(block_pos.size(), BlockType::Air);
                    for (int i = 0 ; i < block_pos.size() ; ++i) {
                        block_types[i] = (BlockType) ((rand() % ((int)BlockType::LAST-1)) + 1);
                    }
                    break;
                }
                case RANDOMIZE:
                    break;
            }

            _client.sendBlockBulkEditPacket(block_pos, block_types);
        }

    private:
        std::vector<glm::ivec3> getBlocksSphere(glm::ivec3 position, int size) {
            std::vector<glm::ivec3> positions;

            int iradius = int(size);
            for (int x = -iradius ; x <= iradius ; ++x) {
            for (int y = -iradius ; y <= iradius ; ++y) {
            for (int z = -iradius ; z <= iradius ; ++z) {
                glm::ivec3 wpos = position + glm::ivec3{x, y, z};
                if (glm::distance2(glm::vec3(position), glm::vec3(wpos)) > size*size) continue;
                positions.push_back(wpos);
            }}}

            return positions;
        }

        std::vector<glm::ivec3> getBlocksCube(glm::ivec3 position, int size) {
            std::vector<glm::ivec3> positions;

            for (int x = -size ; x <= size ; ++x) {
            for (int y = -size ; y <= size ; ++y) {
            for (int z = -size ; z <= size ; ++z) {
                glm::ivec3 wpos = position + glm::ivec3{x, y, z};
                positions.push_back(wpos);
            }}}

            return positions;
        }

    private:
        World& _world;
        Client& _client;
};


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

class Texture {
public:
    Texture(GLsizei width, GLsizei height, GLenum format, GLenum min_filter, GLenum mag_filter, GLenum wrap) {
        _width = width;
        _height = height;

        glCreateTextures(GL_TEXTURE_2D, 1, &_texture);

        glTextureParameteri(_texture, GL_TEXTURE_MIN_FILTER, min_filter);
        glTextureParameteri(_texture, GL_TEXTURE_MAG_FILTER, mag_filter);
        glTextureParameteri(_texture, GL_TEXTURE_WRAP_S, wrap);
        glTextureParameteri(_texture, GL_TEXTURE_WRAP_T, wrap);

        glTextureStorage2D(_texture, 1, format, width, height);
    }
    Texture(GLsizei width, GLsizei height, GLenum format, GLenum min_filter, GLenum mag_filter, GLenum wrap, float borderColor[4]) {
        // create(width, height, format, min_filter, mag_filter, wrap);
        _width = width;
        _height = height;

        glCreateTextures(GL_TEXTURE_2D, 1, &_texture);

        glTextureParameteri(_texture, GL_TEXTURE_MIN_FILTER, min_filter);
        glTextureParameteri(_texture, GL_TEXTURE_MAG_FILTER, mag_filter);
        glTextureParameteri(_texture, GL_TEXTURE_WRAP_S, wrap);
        glTextureParameteri(_texture, GL_TEXTURE_WRAP_T, wrap);
        glTextureParameterfv(_texture, GL_TEXTURE_BORDER_COLOR, borderColor);

        glTextureStorage2D(_texture, 1, format, width, height);
    }


public:
    GLsizei _width, _height;
    GLuint _texture;
};

class Framebuffer {
    public:
        Framebuffer(GLenum draw_buffer, GLenum read_buffer) {
            glCreateFramebuffers(1, &_framebuffer);

            glNamedFramebufferDrawBuffer(_framebuffer, draw_buffer);
            glNamedFramebufferReadBuffer(_framebuffer, read_buffer);
        }

        void bind() {
            glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);
        }

        void attachTexture(GLuint texture, GLenum attachment) {
            glNamedFramebufferTexture(_framebuffer, attachment, texture, 0);
        }

    private:
        GLuint _framebuffer;
};

class GameView: public View {
    public:
        GameView(Context& ctx): View(ctx)
        {
            glfwSetInputMode(ctx.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

            texture_manager.loadAllTextures();

            _depthFBO.attachTexture(_depthTexture._texture, GL_DEPTH_ATTACHMENT);

            cube_shader.use();
            cube_shader.setInt("shadowMap", 0);


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

            const int MAX_NEW_CHUNKS_PER_FRAME = 1;
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

            // 1. render depth of scene to texture (from light's perspective)
            // --------------------------------------------------------------
            glm::mat4 cameraCustomProj = glm::perspective(glm::radians(camera.fov), camera.aspect_ratio, 0.3f, _max_shadow_distance);
            auto corners = getFrustumCornersWorldSpace(cameraCustomProj, camera.getView());

            glm::mat4 lightViewMatrix = getLighViewMatrix(corners, sunDir);
            FrustumBounds bounds = computeFrustumBounds(lightViewMatrix, corners);
            glm::mat4 lightProjectionMatrix = getLightProjectionMatrix(lightViewMatrix, bounds);
            glm::mat4 lightSpaceMatrix = lightProjectionMatrix * lightViewMatrix;
            // https://learn.microsoft.com/en-us/windows/win32/dxtecharts/common-techniques-to-improve-shadow-depth-maps?redirectedfrom=MSDN
            // https://chetanjags.wordpress.com/2015/02/05/real-time-shadows-cascaded-shadow-maps/
            // https://stackoverflow.com/questions/33499053/cascaded-shadow-map-shimmering

            cube_shadowmapping_shader.use();
            cube_shadowmapping_shader.setMat4("u_lightSpaceMatrix", lightSpaceMatrix);
            glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
            _depthFBO.bind();
                glClear(GL_DEPTH_BUFFER_BIT);
                render_scene(cube_shadowmapping_shader);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            // reset viewport
            glViewport(0, 0, ctx.width, ctx.height);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            cube_shader.use();
            cube_shader.setMat4("u_lightSpaceMatrix", lightSpaceMatrix);
            cube_shader.setVec3("u_sun_direction", sunDir);
            cube_shader.setFloat("u_shadow_bias", _shadow_bias);

            glBindTextureUnit(0, _depthTexture._texture);
            render_scene(cube_shader);

            mesh_shader.use();
            mesh_shader.setMat4("u_projectionMatrix", camera.getProjection());
            mesh_shader.setMat4("u_viewMatrix", camera.getView());

            for (auto& entity : world.entities)
            {
                mesh_shader.setMat4("u_modelMatrix", entity.smooth_transform.getMatrix());
                entity.draw();
            }

            // -- Draw debug depth buffer quad -- //
            // glViewport(_width-_width/3, _height-_height/3, _width/3, _height/3);
            // glDisable(GL_DEPTH_TEST);
            // glDisable(GL_CULL_FACE);
            // debugquad_shader.use();
            // debugquad_shader.setInt("depthMap", 0);
            // debugquad_shader.setFloat("near_plane", bounds.minZ);
            // debugquad_shader.setFloat("far_plane", bounds.maxZ);
            // glActiveTexture(GL_TEXTURE0);
            // glBindTexture(GL_TEXTURE_2D, _depthMap);
            // glBindVertexArray(_quadVAO);
            // glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);


            if (_show_debug_gui) {
                gui(dt);
            }
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
                glDrawElements(GL_TRIANGLES, chunk->mesh.indices_count, GL_UNSIGNED_INT, 0);
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

            // ImGui::InputFloat3("Sun direction: ", &sunDir.x, "%.2f");
            ImGui::DragFloat3("Sun direction: ", &sunDir.x, 0.01f, -M_PI*2, M_PI*2, "%.2f");
            ImGui::SliderFloat("Shadow Bias: ", &_shadow_bias, 0.000001f, 0.1f, "%.6f");
            ImGui::SliderFloat("Shadow Distance: ", &_max_shadow_distance, 0.3f, 500.0f, "%.2f");

            const char* const shapes[] {"Sphere", "Cube"};
            ImGui::Combo("Block Shapes: ", (int*)&current_brush_shape, shapes, 2);

            const char* const actions[] {"Place", "Paint", "Paint Randomize"};
            ImGui::Combo("Block Action: ", (int*)&current_brush_action, actions, 3);

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

            if (key == GLFW_KEY_P) {
                _show_debug_gui = !_show_debug_gui;
            }
        }

        // void placeSphere(glm::ivec3 pos, float radius, BlockType blocktype)
        // {
        //     std::vector<glm::ivec3> positions;

        //     int iradius = int(radius);
        //     for (int x = -iradius ; x <= iradius ; ++x) {
        //     for (int y = -iradius ; y <= iradius ; ++y) {
        //     for (int z = -iradius ; z <= iradius ; ++z) {
        //         glm::ivec3 wpos = pos + glm::ivec3{x, y, z};
        //         if (glm::distance2(glm::vec3(pos), glm::vec3(wpos)) > radius*radius) continue;
        //         positions.push_back(wpos);
        //     }
        //     }
        //     }
        //     client.sendBlockBulkEditPacket(positions, blocktype);
        // }



        void onMousePress(int x, int y, int button) {
            if (_show_debug_gui && ImGui::GetIO().WantCaptureMouse) return;

            if (button == GLFW_MOUSE_BUTTON_LEFT) {
                if (ctx.keyState[GLFW_KEY_LEFT_ALT])
                    brush.brush(current_brush_shape, current_brush_action, BlockType::Air, raycastWorldPos, bulkEditRadius);
                else
                    client.sendBreakBlockPacket(raycastWorldPos);
            } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
                if (ctx.keyState[GLFW_KEY_LEFT_ALT])
                    brush.brush(current_brush_shape, current_brush_action, blockInHand, raycastWorldPos, bulkEditRadius);
                else
                    client.sendPlaceBlockPacket(raycastWorldPos + glm::ivec3(raycastNormal), blockInHand);
            }
        }

        void onMouseDrag(int x, int y, int dx, int dy)
        {
            if (_show_debug_gui && ImGui::GetIO().WantCaptureMouse) return;

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
            camera.aspect_ratio = (float)width / (float)height;
        }

    private:
        Program cube_shader{"./assets/shaders/cube.vs", "./assets/shaders/cube.fs"};
        Program cube_shadowmapping_shader{"./assets/shaders/cube_shadowmap.vs", "./assets/shaders/cube_shadowmap.fs"};
        Program mesh_shader{"./assets/shaders/mesh.vs", "./assets/shaders/mesh.fs"};
        Program debugquad_shader{"./assets/shaders/debug_quad.vs", "./assets/shaders/debug_quad_depth.fs"};

        // Shadow map
        const GLsizei SHADOW_WIDTH = 4096, SHADOW_HEIGHT = 4096;
        float borderColor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
        Framebuffer _depthFBO{GL_NONE, GL_NONE};
        Texture _depthTexture{SHADOW_WIDTH, SHADOW_HEIGHT, GL_DEPTH_COMPONENT24, GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_BORDER, borderColor};

        GLuint _quadVAO = 0;
        GLuint _quadVBO;

        glm::vec3 sunDir = glm::normalize(glm::vec3(20.0f, 50.0f, 20.0f));
        float _shadow_bias = 0.000175f;
        // --

        TextureManager texture_manager;

        World world;

        Client client{world, texture_manager, "51.77.194.124"};

        float network_timer = 1.0f;


        bool _cursorEnable = false;
        bool _show_debug_gui = false;
        bool _wireframe = false;
        bool _vsync = true;
        float _max_shadow_distance = 50.0f;

        // Player
        FPSCamera camera = {
            glm::vec3(10.0f, 20.0, 12.0f), 0.0f, 0.0f,
            60.0f, (float)ctx.width / (float)ctx.height, 0.1f, 1000.0f
        };

        Brush brush{world, client};

        Brush::Shape current_brush_shape = Brush::Shape::SPHERE;
        Brush::Action current_brush_action = Brush::Action::PLACE;

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
