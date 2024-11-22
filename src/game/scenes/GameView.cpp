#include "GameView.hpp"

#include "imgui.h"

#include "Chunk.hpp"
#include "Client.hpp"
#include "Entity.hpp"
#include "World.hpp"

#include "command_line_args.h"
#include "string_helpers.h"
#include "ThreadPool.h"
#include "mem_info.h"

#include "clock.h"



GameView::GameView(Context& ctx): View(ctx)
{
    glfwSetInputMode(ctx.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    Client::instance().init(tchat, global_argv[1]);
    Client::instance().Start();
}

void GameView::onHideView()
{
    Client::instance().Stop();
    thread_pool.stop();
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

    player_blockraycasthit = World::instance().blockRaycast(camera.getPosition(), camera.forward(), 16);

    network_timer -= dt;
    if (network_timer <= 0.0f) {
        network_timer = 1.0f / 20.0f;
        networkUpdate();
    }
}

void GameView::update3x3Chunks(const glm::ivec3& chunk_pos)
{
    // constexpr glm::ivec3 offsets[] = { {0, 0, 0}, {-1, 0, 0}, {1, 0, 0}, {0, -1, 0}, {0, 1, 0}, {0, 0, -1}, {0, 0, 1} }; // center + adjacents
    // constexpr glm::ivec3 offsets[] = { {0, 0, 0} }; // center

    for (int z = -1 ; z <= 1; ++z) {
    for (int y = -1 ; y <= 1; ++y) {
    for (int x = -1 ; x <= 1; ++x) {
    // for (const glm::ivec3 &offset: offsets) {
        const glm::ivec3 offset = {x, y, z};
        if (Chunk* neighbor_chunk = World::instance().getChunk(chunk_pos + offset)) {

            ChunkMesh new_chunk_mesh;
            new_chunk_mesh.computeVertexBuffer(neighbor_chunk);

            main_task_queue.push_safe([&, neighbor_chunk, new_chunk_mesh]() mutable {
                auto old_mesh = neighbor_chunk->mesh;
                new_chunk_mesh.updateVAO(world_renderer.buffer_allocator_vertices, world_renderer.buffer_allocator_indices, old_mesh.slot_vertices, old_mesh.slot_indices);
                neighbor_chunk->mesh = new_chunk_mesh;

                // if (old_mesh.slot_vertices.id != -1)
                //     world_renderer.buffer_allocator_vertices.deallocate(old_mesh.slot_vertices.id);
                // if (old_mesh.slot_indices.id != -1)
                //     world_renderer.buffer_allocator_indices.deallocate(old_mesh.slot_indices.id);
            });
        }
    }
    }
    }
}

void GameView::consumeNewChunks()
{
    main_task_queue.execute();

    const std::lock_guard<std::mutex> lock(Client::instance().new_chunks_mutex);

    // NOTE: the chunks are sent to be queued before having the chance to be sorted by distance (the solution is to sort the chunks on the server)
    // const glm::vec3 camPos = camera.getPosition();
    // std::sort(Client::instance().new_chunks.begin(), Client::instance().new_chunks.end(),
    //     [camPos](const Packet::Server::ChunkPacket* l, const Packet::Server::ChunkPacket* r)
    //     {
    //         return glm::distance2(camPos, glm::vec3(l->pos*16)) > glm::distance2(camPos, glm::vec3(r->pos*16));
    //     });

    // TODO: instead of updating neigbours chunks directly, set the chunks all at once and add to an unordered_map the chunks to update the mesh, then dispatch all thoses

    while (Client::instance().new_chunks.size() > 0) {

        Packet::Server::ChunkPacket* chunk_data = Client::instance().new_chunks.back();
        Client::instance().new_chunks.pop_back();

        thread_pool.enqueue([this, chunk_data] {
            Chunk* chunk = World::instance().setChunk(chunk_data);
            if (chunk == nullptr) {
                return;
            }

            delete chunk_data;

            update3x3Chunks(chunk->pos);
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
    world_renderer.render(camera);

    ctx.imguiNewFrame();
    if (_show_debug_gui) gui(dt);
    ctx.imguiRender();

}

void GameView::gui(float dt)
{
    // ImGui::ShowDemoWindow();

    // ImGui::Begin("Shadow map");
    // ImGui::Image((ImTextureID)(intptr_t)shadowmap._depthTexture->_texture, ImVec2(ctx.width/3, ctx.height/3), ImVec2(0, 1), ImVec2(1, 0));
    // ImGui::End();

    ImGui::Begin("Debug");

    ImGui::Text("%s", SimpleProfiler::instance().dump().c_str());

    ImGui::Text("RAM: %.3f / %.3f Go", ((double)getCurrentRSS()) / (1024*1024*1024), ((double)getPeakRSS()) / (1024*1024*1024));

    ImGui::Text("New chunks: %ld", Client::instance().new_chunks.size());
    ImGui::Text("Thread pool tasks %ld", thread_pool._task_queue.size());

    ImGui::Text("Chunks: %d (%d rendered)", World::instance().getChunkCount(), world_renderer.chunks_drawn);

    ImGui::Text("%.4f secs", dt);
    ImGui::Text("%.2f fps", 1.0f / dt);

    glm::vec3 camera_pos = camera.getPosition();
    ImGui::Text("position: %.2f, %.2f, %.2f", camera_pos.x, camera_pos.y, camera_pos.z);
    ImGui::Text("forward: %.2f, %.2f, %.2f", camera.forward().x, camera.forward().y, camera.forward().z);
    ImGui::Text("block in hand: %d", (int)block_in_hand);

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


    ImGui::SliderFloat("Bulk Edit Radius: ", &bulk_edit_radius, 1.0f, 32.0f, "%.2f");
    ImGui::Checkbox("Wireframe", &world_renderer._wireframe);
    ImGui::Checkbox("Ambient occlusion", &world_renderer._ambient_occlusion);
    ImGui::SliderFloat("AO strength: ", &world_renderer._ambient_occlusion_strength, 0.0f, 1.0f, "%.2f");

    if (ImGui::Checkbox("VSync", &_vsync)) {
        ctx.setVsync(_vsync);
    }

    ImGui::DragFloat3("Sun direction: ", &world_renderer.sunDir.x, 0.01f, -M_PI*2, M_PI*2, "%.2f");
    ImGui::SliderFloat("Shadow Bias: ", &world_renderer.shadowmap._shadow_bias, 0.000001f, 0.001f, "%.6f");
    ImGui::SliderFloat("Shadow Distance: ", &world_renderer.shadowmap._max_shadow_distance, 0.3f, 500.0f, "%.2f");

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_HorizontalScrollbar;
    // if (disable_mouse_wheel)
    //     window_flags |= ImGuiWindowFlags_NoScrollWithMouse;
    ImGui::BeginChild("ChildL", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 260), ImGuiChildFlags_None, window_flags);
    for (const auto& msg: tchat) {
        ImGui::TextWrapped("%s", msg.c_str());
        ImGui::Spacing();
    }

    ImGui::EndChild();

    ImGui::InputText("##inputText", input_text_buffer, IM_ARRAYSIZE(input_text_buffer));
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
            placeSphere(player_blockraycasthit.pos, bulk_edit_radius, BlockType::Air);
        else
            Client::instance().sendBreakBlockPacket(player_blockraycasthit.pos);
    } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (ctx.keystate[GLFW_KEY_LEFT_ALT])
            placeSphere(player_blockraycasthit.pos, bulk_edit_radius, block_in_hand);
        else
            Client::instance().sendPlaceBlockPacket(player_blockraycasthit.pos + glm::ivec3(player_blockraycasthit.normal), block_in_hand);
    }
}

void GameView::onMouseDrag(int x, int y, int dx, int dy)
{
    if (_show_debug_gui && ImGui::GetIO().WantCaptureMouse) return;
}

void GameView::onMouseScroll(int scroll_x, int scroll_y)
{
    int block = ((int)block_in_hand + scroll_y) % ((int)BlockType::LAST-1);
    if (block < 1)
        block += (int)BlockType::LAST-1;
    block_in_hand = (BlockType)block;
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
