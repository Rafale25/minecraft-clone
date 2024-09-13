#include "game_view.hpp"

#include "context.hpp"

#include "view.hpp"
#include "camera.hpp"
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
#include "ServerPacket.hpp"
// #include "Tchat.hpp"

#include "imgui.h"
#include <algorithm>
#include <string>

#include "command_line_args.h"
#include "string_helpers.hpp"

#include "Frustum.hpp"
#include "thread_pool.hpp"

#include "mem_info.hpp"

void update3x3Chunks(const glm::ivec3& chunk_pos, TaskQueue& main_task_queue)
{
    const glm::ivec3 offsets[] = { {0, 0, 0}, {-1, 0, 0}, {1, 0, 0}, {0, -1, 0}, {0, 1, 0}, {0, 0, -1}, {0, 0, 1} };

    for (const glm::ivec3 &offset: offsets) {
        if (Chunk* neighbor_chunk = World::instance().getChunk(chunk_pos + offset)) {

            ChunkMesh new_chunk_mesh;
            new_chunk_mesh.computeVertexBuffer(neighbor_chunk);

            main_task_queue.push_safe([neighbor_chunk, new_chunk_mesh]() mutable {
                new_chunk_mesh.updateVAO();

                auto old_mesh = neighbor_chunk->mesh;
                // neighbor_chunk->mesh.deleteAll(); // this being before the assignation is a potential race condition (could cause a segfault is the render try to use the variable)
                neighbor_chunk->mesh = new_chunk_mesh;

                old_mesh.deleteAll();
            });
        }
    }
}

GameView::GameView(Context& ctx): View(ctx)
{
    glfwSetInputMode(ctx.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    TextureManager::loadAllTextures();
    ssbo_texture_handles = createBufferStorage(&TextureManager::Get().textures_handles[0], TextureManager::Get().textures_handles.size() * sizeof(GLuint64));

    cube_shader.use();
    cube_shader.setInt("shadowMap", 0);


    Client::instance().init(tchat, global_argv[1]);
    Client::instance().Start();
}

void GameView::onUpdate(double time_since_start, float dt)
{

    glm::vec3 delta = {
        ctx.keystate[GLFW_KEY_A] - ctx.keystate[GLFW_KEY_D],
        ctx.keystate[GLFW_KEY_LEFT_CONTROL] - ctx.keystate[GLFW_KEY_SPACE],
        ctx.keystate[GLFW_KEY_W] - ctx.keystate[GLFW_KEY_S]
    };

    camera.setSpeed(
        ctx.keystate[GLFW_KEY_LEFT_SHIFT] == GLFW_PRESS ? 130.0f : 10.0f
    );

    if (!_cursor_enabled) camera.move(delta);
    camera.update(dt);

    consumeTaskQueue();
    consumeNewChunks();

    World::instance().updateEntities();

    player_blockraycasthit = World::instance().BlockRaycast(camera.getPosition(), camera.forward(), 16);

    network_timer -= dt;
    if (network_timer <= 0.0f) {
        network_timer = 1.0f / 20.0f;
        networkUpdate();
    }
}

void GameView::consumeNewChunks()
{
    {
        // TODO: Don't understand why i can't pop an element from the task queue.
        // Using the auto for loop for the moment because it works.
        const std::lock_guard<std::mutex> lock(main_task_queue._task_queue_mutex);
        for (auto &task: main_task_queue._task_queue) {
            task();
        }
        main_task_queue._task_queue.clear();
    }

    const std::lock_guard<std::mutex> lock(Client::instance().new_chunks_mutex);

    // const glm::vec3 camPos = camera.getPosition();
    // std::sort(client.new_chunks.begin(), client.new_chunks.end(),
    //     [this, camPos](const Packet::Server::ChunkPacket* l, const Packet::Server::ChunkPacket* r)
    //     {
    //         return glm::distance2(camPos, glm::vec3(l->pos*16)) > glm::distance2(camPos, glm::vec3(r->pos*16));
    //     });

    while (Client::instance().new_chunks.size() > 0) {
        Packet::Server::ChunkPacket* chunk_data = Client::instance().new_chunks.back();
        Client::instance().new_chunks.pop_back();

        thread_pool.enqueue([this, chunk_data] {
            Chunk* chunk = World::instance().setChunk(chunk_data);

            delete chunk_data;

            update3x3Chunks(chunk->pos, main_task_queue);
        });
    }
}

void GameView::consumeTaskQueue()
{
    const std::lock_guard<std::mutex> lock(Client::instance().task_queue_mutex);

    for (auto &task: Client::instance().task_queue) {
        task();
    }
    Client::instance().task_queue.clear();
}

void GameView::networkUpdate()
{
    if (Client::instance().client_id == -1) return;

    glm::vec3 pos = camera.getPosition();
    float yaw = camera.getYaw();
    float pitch = camera.getPitch();

    Client::instance().sendUpdateEntityPacket(pos, yaw, pitch);
}

void GameView::onDraw(double time_since_start, float dt)
{
    glEnable(GL_MULTISAMPLE); // enabled by default

    glPolygonMode(GL_FRONT_AND_BACK, _wireframe ? GL_LINE : GL_FILL);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);

    // glEnable(GL_FRAMEBUFFER_SRGB);

    glClearColor(135.0f/255.0f, 206.0f/255.0f, 250.0f/255.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // ShadowMap //
    shadowmap.setSunDir(sunDir);
    shadowmap.begin(camera, cube_shadowmapping_shader);
        render_world(cube_shadowmapping_shader, false);
    shadowmap.end();

    // Skybox //
    glDisable(GL_DEPTH_TEST);
    skybox_shader.use();
    glm::mat4 view_rotation = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), -camera.forward(), glm::vec3(0.0f, 1.0f, 0.0f)); // wtf
    skybox_shader.setVec2("u_resolution", glm::vec2(ctx.width, ctx.height));
    skybox_shader.setMat4("u_view", view_rotation);
    skybox_shader.setFloat("u_sunDotAngle", glm::dot(sunDir, {0.0f, 1.0f, 0.0f}));
    skybox_quad.draw();
    glEnable(GL_DEPTH_TEST);

    // Terrain //
    cube_shader.use();
    cube_shader.setMat4("u_lightSpaceMatrix", shadowmap._lightSpaceMatrix);
    cube_shader.setVec3("u_sun_direction", sunDir);
    cube_shader.setFloat("u_shadow_bias", shadowmap._shadow_bias);
    glBindTextureUnit(0, shadowmap._depthTexture._texture);
    render_world(cube_shader);

    // Entities //
    mesh_shader.use();
    mesh_shader.setMat4("u_projectionMatrix", camera.getProjection());
    mesh_shader.setMat4("u_viewMatrix", camera.getView());
    for (auto& entity : World::instance().entities)
    {
        mesh_shader.setMat4("u_modelMatrix", entity.smooth_transform.getMatrix());
        entity.draw();
    }

    ctx.imguiNewFrame();
    if (_show_debug_gui) gui(dt);
    ctx.imguiRender();
}

void GameView::render_world(const Program &shader, bool use_frustum_culling)
{
    shader.use();
    shader.setMat4("u_projectionMatrix", camera.getProjection());
    shader.setMat4("u_viewMatrix", camera.getView());
    shader.setVec3("u_view_position", camera.getPosition());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo_texture_handles);

    Frustum camera_frustum = createFrustumFromCamera(camera, camera.aspect_ratio, glm::radians(camera.fov), camera.near_plane, camera.far_plane);

    _chunks_drawn = 0;

    const std::shared_lock<std::shared_mutex> lock(World::instance().chunks_mutex);

    for (const auto& [key, chunk] : World::instance().chunks)
    {
        if (chunk->mesh.indices_count == 0 || chunk->mesh.VAO == 0) continue;

        if (use_frustum_culling) {
            AABB chunk_aabb = {(chunk->pos * 16), (chunk->pos * 16) + 16};
            if (!chunk_aabb.isOnFrustum(camera_frustum)) continue;
        }

        shader.setVec3("u_chunkPos", chunk->pos * 16);

        glBindVertexArray(chunk->mesh.VAO);

        // printf("indices_count: %d; VAO: %d\n", chunk->mesh.indices_count, chunk->mesh.VAO);
        glDrawElements(GL_TRIANGLES, chunk->mesh.indices_count, GL_UNSIGNED_INT, 0);
        ++_chunks_drawn;
    }
}

void GameView::gui(float dt)
{
    // ImGui::ShowDemoWindow();

    // ImGui::Begin("Shadow map");
    // ImGui::Image((ImTextureID)(intptr_t)shadowmap._depthTexture->_texture, ImVec2(ctx.width/3, ctx.height/3), ImVec2(0, 1), ImVec2(1, 0));
    // ImGui::End();

    ImGui::Begin("Debug");

    ImGui::Text("RAM: %.3f / %.3f Go", ((double)getCurrentRSS()) / (1024*1024*1024), ((double)getPeakRSS()) / (1024*1024*1024));

    ImGui::Text("new chunks: %ld", Client::instance().new_chunks.size());
    ImGui::Text("thread pools tasks %ld", thread_pool._task_queue.size());

    ImGui::Text("draw calls: %d", _chunks_drawn);

    ImGui::Text("%.4f secs", dt);
    ImGui::Text("%.2f fps", 1.0f / dt);

    glm::vec3 camera_pos = camera.getPosition();
    ImGui::Text("position: %.2f, %.2f, %.2f", camera_pos.x, camera_pos.y, camera_pos.z);
    ImGui::Text("forward: %.2f, %.2f, %.2f", camera.forward().x, camera.forward().y, camera.forward().z);
    ImGui::Text("block in hand: %d", (int)blockInHand);

    ImGui::Text("ClientId: %d", Client::instance().client_id);

    if (ImGui::TreeNode(SC("Entities: " << World::instance().entities.size()))) {
        for (auto& entity : World::instance().entities) {
            ImGui::PushID(entity.id);
            ImGui::Text("id:%d: x:%.2f y:%.2f z:%.2f", entity.id, entity.transform.position.x, entity.transform.position.y, entity.transform.position.z);
            ImGui::Text("name: %s", entity.name.c_str());
            ImGui::SameLine();
            if (ImGui::Button("teleport")) {
                setPlayerPosition(entity.transform.position);
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

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_HorizontalScrollbar;
    // if (disable_mouse_wheel)
    //     window_flags |= ImGuiWindowFlags_NoScrollWithMouse;
    ImGui::BeginChild("ChildL", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 260), ImGuiChildFlags_None, window_flags);
    for (const auto& msg: tchat) {
        ImGui::TextWrapped("%s", msg.c_str());
        ImGui::Spacing();
    }
    ImGui::EndChild();

    ImGui::InputText("", input_text_buffer, IM_ARRAYSIZE(input_text_buffer));
    ImGui::SameLine();
    if (ImGui::Button("Send")) {
        sendTextMessage();
    }
    ImGui::End();
}

void GameView::sendTextMessage() {
    if (strlen(input_text_buffer) <= 0) return;
    Client::instance().sendChatMessagePacket(input_text_buffer);
    memset(input_text_buffer, 0, sizeof(input_text_buffer));
}

void GameView::placeSphere(const glm::ivec3& center, float radius, BlockType blocktype)
{
    std::vector<glm::ivec3> positions;

    int iradius = int(radius);
    for (int x = -iradius ; x <= iradius ; ++x) {
    for (int y = -iradius ; y <= iradius ; ++y) {
    for (int z = -iradius ; z <= iradius ; ++z) {
        glm::ivec3 p = center + glm::ivec3{x, y, z};
        if (glm::distance2(glm::vec3(center), glm::vec3(p)) > radius*radius) continue;
        positions.push_back(p);
    }
    }
    }
    Client::instance().sendBlockBulkEditPacket(positions, blocktype);
}

void GameView::setPlayerPosition(const glm::vec3& p) {
    camera.setPosition(p);
}

void GameView::onKeyPress(int key)
{
    if (key == GLFW_KEY_C) {
        _cursor_enabled = !_cursor_enabled;

        if (_cursor_enabled)
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

    if (!ImGui::GetIO().WantCaptureKeyboard) {
        if (key == GLFW_KEY_P) {
            _show_debug_gui = !_show_debug_gui;
        }
    }
}

void GameView::onMousePress(int x, int y, int button) {
    if (_show_debug_gui && ImGui::GetIO().WantCaptureMouse) return;

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (ctx.keystate[GLFW_KEY_LEFT_ALT])
            placeSphere(player_blockraycasthit.pos, bulkEditRadius, BlockType::Air);
        else
            Client::instance().sendBreakBlockPacket(player_blockraycasthit.pos);
    } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (ctx.keystate[GLFW_KEY_LEFT_ALT])
            placeSphere(player_blockraycasthit.pos, bulkEditRadius, blockInHand);
        else
            Client::instance().sendPlaceBlockPacket(player_blockraycasthit.pos + glm::ivec3(player_blockraycasthit.normal), blockInHand);
    }
}

void GameView::onMouseDrag(int x, int y, int dx, int dy)
{
    if (_show_debug_gui && ImGui::GetIO().WantCaptureMouse) return;
}

void GameView::onMouseScroll(int scroll_x, int scroll_y)
{
    int block = ((int)blockInHand + scroll_y) % ((int)BlockType::LAST-1);
    if (block < 1)
        block += (int)BlockType::LAST-1;
    blockInHand = (BlockType)block;
}

void GameView::onMouseMotion(int x, int y, int dx, int dy)
{
    if (!_cursor_enabled)
        camera.onMouseMotion(x, y, dx, dy);
}

void GameView::onResize(int width, int height)
{
    glViewport(0, 0, width, height);
    camera.aspect_ratio = (float)width / (float)height;
}
