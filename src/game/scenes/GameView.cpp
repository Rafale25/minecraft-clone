#include "GameView.hpp"

#include "imgui.h"

#include "Chunk.hpp"
#include "Client.hpp"
#include "Entity.hpp"
#include "World.hpp"

#include "command_line_args.h"
#include "string_helpers.h"
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
    world_renderer.thread_pool.stop();
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

    Client::instance().task_queue.execute();

    processNewChunks();
    deleteFarChunks();

    world_renderer.update();

    World::instance().updateEntities();

    player_blockraycasthit = World::instance().blockRaycast(camera.getPosition(), camera.forward(), 16);

    network_timer -= dt;
    if (network_timer <= 0.0f) {
        network_timer = 1.0f / 20.0f;
        networkUpdate();
    }
}

void GameView::deleteFarChunks()
{
    const std::lock_guard<std::shared_mutex> lock(World::instance().chunks_mutex);

    std::vector<glm::ivec3> pos_to_delete;

    auto& world_chunks = World::instance().chunks;
    for (const auto& [pos, chunk] : world_chunks) {
        const float camera_chunk_dist = glm::distance(camera.getPosition(), glm::vec3(chunk->pos) * 16.0f);
        if (camera_chunk_dist > world_renderer.chunk_view_distance + world_renderer.chunk_delete_offset) {
            pos_to_delete.push_back(pos);
        }
    }

    for (const auto &pos : pos_to_delete) {
        World::instance().deleteChunk(pos);
        world_renderer.onDeletedChunk(pos);
    }
}

void GameView::processNewChunks()
{
    const std::lock_guard<std::mutex> lock(Client::instance().new_chunks_mutex);

    while (Client::instance().new_chunks.size() > 0) {

        Packet::Server::ChunkPacket* chunk_data = Client::instance().new_chunks.back();
        Client::instance().new_chunks.pop_back();

        // Don't process imcoming chunk out of render distance
        const float camera_chunk_dist = glm::distance(camera.getPosition(), glm::vec3(chunk_data->pos) * 16.0f);
        if (camera_chunk_dist > world_renderer.chunk_view_distance + world_renderer.chunk_delete_offset) {
            delete chunk_data;
            continue;
        }

        const Chunk* chunk = World::instance().setChunk(chunk_data->pos, chunk_data->blocks);
        if (chunk) {
            world_renderer.onAddedChunk(chunk_data->pos);
        }
        delete chunk_data;
    }

}

void GameView::networkUpdate()
{
    Client::instance().sendUpdateEntityPacket(camera.getPosition(), camera.getYaw() + M_PI, M_PI_2 - camera.getPitch() - M_PI_2 );
}

void GameView::onDraw(double time_since_start, float dt)
{
    world_renderer.render(camera);

    ctx.imguiNewFrame();
    if (_show_debug_gui) gui(dt);
    ctx.imguiRender();
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
    int block = ((int)block_in_hand + scroll_y) % ((int)BlockType::INVALID-1);
    if (block < 1)
        block += (int)BlockType::INVALID-1;
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

    world_renderer.onResize(width, height);
}

void GameView::gui(float dt)
{
    // ImGui::ShowDemoWindow();

    // ImGui::Begin("Shadow map");
    // ImGui::Image((ImTextureID)(intptr_t) world_renderer.shadowmap._depthTexture._texture, ImVec2(ctx.width/3, ctx.height/3), ImVec2(0, 1), ImVec2(1, 0));
    // ImGui::End();

    ImGui::Begin("Debug");

    ImGui::Text("%s", SimpleProfiler::instance().dump().c_str());

    ImGui::Text("RAM: %.4f / %.4f Go", ((double)getCurrentRSS()) / (1024*1024*1024), ((double)getPeakRSS()) / (1024*1024*1024));

    ImGui::Text("BufferVertices: %d / %d", world_renderer.buffer_allocator_vertices.getFreeSlotsCount(), world_renderer.buffer_allocator_vertices.getMaxSlotsCount());
    ImGui::Text("BufferIndices: %d / %d", world_renderer.buffer_allocator_indices.getFreeSlotsCount(), world_renderer.buffer_allocator_indices.getMaxSlotsCount());

    ImGui::Text("New chunks: %ld", Client::instance().new_chunks.size());
    ImGui::Text("ThreadPool{%lu} tasks: %ld", world_renderer.thread_pool._workers.size(), world_renderer.thread_pool._task_queue.size());

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

    ImGui::BeginChild("ChildL", ImVec2(ImGui::GetContentRegionAvail().x * 0.5f, 260), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar);
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


// // #Sort Chunks
// NOTE: the chunks are sent to be queued before having the chance to be sorted by distance (the solution is to sort the chunks on the server)
// const glm::vec3 camPos = camera.getPosition();
// std::sort(Client::instance().new_chunks.begin(), Client::instance().new_chunks.end(),
//     [camPos](const Packet::Server::ChunkPacket* l, const Packet::Server::ChunkPacket* r)
//     {
//         return glm::distance2(camPos, glm::vec3(l->pos*16)) > glm::distance2(camPos, glm::vec3(r->pos*16));
//     });
