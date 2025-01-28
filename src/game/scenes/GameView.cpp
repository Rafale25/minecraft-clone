#include "GameView.hpp"

#include "imgui.h"

#include "World.hpp"
#include "Chunk.hpp"
#include "Client.hpp"
#include "Entity.hpp"
// #include "FPSCamera.hpp"
#include "AABB.hpp"

#include "world_to_screen_space.h"
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
    playerMovements(dt);
    camera.update(dt);

    Client::instance().task_queue.execute();

    processNewChunks();
    if (_delete_far_chunks) deleteFarChunks();

    world_renderer.update();

    World::instance().updateEntities();

    player_blockraycasthit = World::instance().blockRaycast(camera.getPosition(), camera.forward(), 16);

    network_timer -= dt;
    if (network_timer <= 0.0f) {
        network_timer = 1.0f / 20.0f;
        networkUpdate();
    }

    if (player_blockraycasthit.blocktype != BlockType::Air) {
        DebugDraw::instance().drawCube(player_blockraycasthit.world_pos + 0.5f, 1.0f, {0.8f, 0.8f, 0.8f});
    }

    if (_show_debug_gui) {
        for (const auto& e : World::instance().entities) {
            DebugDraw::instance().drawCuboid(e.transform.position, {0.3f, 1.0f, 0.3f});
        }
    }

    if (_draw_chunks_borders) {
        for (const auto& [pos, chunk] : World::instance().chunks) {
            DebugDraw::instance().drawCube(glm::vec3(pos * 16) + glm::vec3(8.0f), 16.0f);
        }
    }

    if (_draw_player_chunk) {
        for (int i = 0; i <= 16 ; ++i) {
            const glm::vec3 py00 = glm::floor(camera.getPosition() / 16.0f) * 16.0f + glm::vec3(0.0f, (float)i, 0.0f);
            const glm::vec3 py11 = glm::floor(camera.getPosition() / 16.0f) * 16.0f + glm::vec3(16.0f, (float)i, 16.0f);
            const glm::vec3 pz00 = glm::floor(camera.getPosition() / 16.0f) * 16.0f + glm::vec3((float)i, 0.0f, 0.0f);
            const glm::vec3 pz11 = glm::floor(camera.getPosition() / 16.0f) * 16.0f + glm::vec3((float)i, 16.0f, 16.0f);
            const glm::vec3 px00 = glm::floor(camera.getPosition() / 16.0f) * 16.0f + glm::vec3(0.0f, 0.0f, (float)i);
            const glm::vec3 px11 = glm::floor(camera.getPosition() / 16.0f) * 16.0f + glm::vec3(16.0f, 16.0f, (float)i);

            DebugDraw::instance().drawLine(py00, py00 + glm::vec3{16, 0, 0});
            DebugDraw::instance().drawLine(py00, py00 + glm::vec3{0, 0, 16});
            DebugDraw::instance().drawLine(py11, py11 + glm::vec3{-16, 0, 0});
            DebugDraw::instance().drawLine(py11, py11 + glm::vec3{0, 0, -16});

            DebugDraw::instance().drawLine(pz00, pz00 + glm::vec3{0, 16, 0});
            DebugDraw::instance().drawLine(pz00, pz00 + glm::vec3{0, 0, 16});
            DebugDraw::instance().drawLine(pz11, pz11 + glm::vec3{0, 0, -16});
            DebugDraw::instance().drawLine(pz11, pz11 + glm::vec3{0, -16, 0});

            DebugDraw::instance().drawLine(px00, px00 + glm::vec3{0, 16, 0});
            DebugDraw::instance().drawLine(px00, px00 + glm::vec3{16, 0, 0});
            DebugDraw::instance().drawLine(px11, px11 + glm::vec3{-16, 0, 0});
            DebugDraw::instance().drawLine(px11, px11 + glm::vec3{0, -16, 0});
        }

        for (int z = - 1 ; z <= 1 ; ++z) {
        for (int x = - 1 ; x <= 1 ; ++x) {
            const glm::vec3 p = glm::floor(camera.getPosition() / 16.0f) * 16.0f + glm::vec3(x, 0, z) * 16.0f;
            DebugDraw::instance().drawLine(p + glm::vec3(0, -128, 0), p + glm::vec3{0, 128, 0}, {1, 0, 1});
            DebugDraw::instance().drawLine(p + glm::vec3(16, -128, 0), p + glm::vec3{16, 128, 0}, {1, 0, 1});
            DebugDraw::instance().drawLine(p + glm::vec3(0, -128, 16), p + glm::vec3{0, 128, 16}, {1, 0, 1});
            DebugDraw::instance().drawLine(p + glm::vec3(16, -128, 16), p + glm::vec3{16, 128, 16}, {1, 0, 1});
        }
        }
    }
}

void GameView::playerMovements(float dt)
{
    glm::vec3 delta = {
        ctx.keystate[GLFW_KEY_A] - ctx.keystate[GLFW_KEY_D],
        ctx.keystate[GLFW_KEY_LEFT_CONTROL] - ctx.keystate[GLFW_KEY_SPACE],
        ctx.keystate[GLFW_KEY_W] - ctx.keystate[GLFW_KEY_S]
    };

    if (_cursor_enabled || ImGui::GetIO().WantCaptureKeyboard) return;

    if (free_cam) {
        camera.setSpeed(
            ctx.keystate[GLFW_KEY_LEFT_SHIFT] == GLFW_PRESS ? 130.0f : 10.0f
        );

        if (!_cursor_enabled && !ImGui::GetIO().WantCaptureKeyboard) camera.move(delta);
        return;
    }


    glm::vec3 forward_xz = glm::normalize(glm::vec3(camera.forward().x, 0.0f, camera.forward().z));
    glm::vec3 move_vector = -delta.x * camera.right() + delta.z * forward_xz;

    glm::vec3 player_feet_position = camera.getPosition() - glm::vec3(0.0f, player_height, 0.0f);

    printf("Y: %.6f %.6f %.6f\n", glm::mod(player_feet_position.x, 1.0f), glm::mod(player_feet_position.y, 1.0f), glm::mod(player_feet_position.z, 1.0f));
    // printf("Y: %.5f\n", glm::mod(player_feet_position.y, 1.0f));

    AABB player_aabb_under_feet = {
        player_feet_position + glm::vec3(-player_radius+0.01f, -0.01f, -player_radius+0.01f),
        player_feet_position + glm::vec3(player_radius-0.01f, 0.01f, player_radius-0.01f)
    };

    if (_draw_player_colliders) DebugDraw::instance().drawCuboidMinMax(player_aabb_under_feet.min, player_aabb_under_feet.max, {0.4f, 0.2, 0.8});

    std::vector<AABB> neighbours_blocks_AABB;
    for (int z = -2 ; z <= 2 ; ++z) {
    for (int y = -2 ; y <= 2 ; ++y) {
    for (int x = -2 ; x <= 2 ; ++x) {
        glm::vec3 p = glm::floor(player_feet_position + glm::vec3(x, y, z));
        if (World::instance().getBlock(p) != BlockType::Air) {
            neighbours_blocks_AABB.push_back(AABB(p, p + 1.0f));

            if (_draw_player_colliders) DebugDraw::instance().drawCuboidMinMax(p, p + 1.0f);
        }
    }}}

    bool is_grounded = false;
    for (const auto& aabb : neighbours_blocks_AABB) {
        bool collide = AABB::AABBtoAABB(aabb, player_aabb_under_feet);
        if (collide) {
            is_grounded = true;
            break;
        }
    }

    if (is_grounded) {
        player_velocity.x *= 0.8f;
        player_velocity.z *= 0.8f;
    }
    if (is_grounded && ctx.keystate[GLFW_KEY_SPACE]) {
        player_velocity.y += 12.0f;
    }

    player_velocity.y -= player_gravity * dt;
    player_velocity.y = glm::clamp(player_velocity.y, -40.0f, 40.0f);

    if (is_grounded) {
        player_velocity += move_vector * 1.0f;
    } else {
        player_velocity += move_vector * 0.1f;
    }

    glm::vec3 next_pos = player_feet_position + player_velocity * dt;

    // TODO: find difference in AABB X and use it to place player perfectly against block

    AABB player_aabb_y = {
        glm::vec3(player_feet_position.x, next_pos.y, player_feet_position.z) + glm::vec3(-0.3f, 0.0f, -0.3f),
        glm::vec3(player_feet_position.x, next_pos.y, player_feet_position.z) + glm::vec3(0.3f, player_height, 0.3f)};
    if (_draw_player_colliders) DebugDraw::instance().drawCuboidMinMax(player_aabb_y.min, player_aabb_y.max, {1.0f, 0.2, 0.8});

    // Y
    for (const auto& aabb : neighbours_blocks_AABB) {
        if (AABB::AABBtoAABB(aabb, player_aabb_y)) {
            float dy = AABB::AABBtoAABBOverlapDistance(aabb, player_aabb_y).y;
            next_pos.y += dy;
            player_velocity.y = 0.0f;
            break;
        }
    }

    AABB player_aabb_x = {
        glm::vec3(next_pos.x, next_pos.y, player_feet_position.z) + glm::vec3(-0.3f, 0.0f, -0.3f),
        glm::vec3(next_pos.x, next_pos.y, player_feet_position.z) + glm::vec3(0.3f, player_height, 0.3f)};

    // X
    for (const auto& aabb : neighbours_blocks_AABB) {
        if (AABB::AABBtoAABB(aabb, player_aabb_x)) {
            float dx = AABB::AABBtoAABBOverlapDistance(aabb, player_aabb_x).x;
            next_pos.x += dx;
            player_velocity.x = 0.0f;
            break;
        }
    }

    AABB player_aabb_z = {
        glm::vec3(next_pos.x, next_pos.y, next_pos.z) + glm::vec3(-0.3f, 0.0f, -0.3f),
        glm::vec3(next_pos.x, next_pos.y, next_pos.z) + glm::vec3(0.3f, player_height, 0.3f)};

    // Z
    for (const auto& aabb : neighbours_blocks_AABB) {
        if (AABB::AABBtoAABB(aabb, player_aabb_z)) {
            float dz = AABB::AABBtoAABBOverlapDistance(aabb, player_aabb_z).z;
            next_pos.z += dz;
            player_velocity.z = 0.0f;
            break;
        }
    }


    camera.setPosition(next_pos + glm::vec3(0.0f, player_height, 0.0f));
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
    Client::instance().sendUpdateEntityPacket(camera.getPosition(), camera.getYaw() + std::numbers::pi, - camera.getPitch());
}

void GameView::onDraw(double time_since_start, float dt)
{
    world_renderer.render(camera);

    ctx.imguiNewFrame();
    if (_show_debug_gui) gui(dt);
    drawPlayersNames();

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
            placeSphere(player_blockraycasthit.block_pos, bulk_edit_radius, BlockType::Air);
        else
            Client::instance().sendBreakBlockPacket(player_blockraycasthit.block_pos);
    } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (ctx.keystate[GLFW_KEY_LEFT_ALT])
            placeSphere(player_blockraycasthit.block_pos, bulk_edit_radius, block_in_hand);
        else {
            if (player_blockraycasthit.blocktype != BlockType::Air) {
                Client::instance().sendPlaceBlockPacket(player_blockraycasthit.block_pos + glm::ivec3(player_blockraycasthit.normal), block_in_hand);
            }
        }
    }

    // Pick block
    if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
        BlockType block = World::instance().blockRaycast(camera.getPosition(), camera.forward(), 64).blocktype;
        block_in_hand = block;
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

void GameView::drawPlayersNames()
{
    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoTitleBar;
    window_flags |= ImGuiWindowFlags_NoScrollbar;
    window_flags |= ImGuiWindowFlags_NoResize;
    window_flags |= ImGuiWindowFlags_NoCollapse;
    window_flags |= ImGuiWindowFlags_NoNav;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
    window_flags |= ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoInputs;
    // window_flags |= ImGuiWindowFlags_NoBackground;

    int idx = 0;
    for (const Entity& e : World::instance().entities) {

        glm::ivec2 screen_pos = worldToScreenSpace(e.smooth_transform.position + glm::vec3(0.0f, 0.8f, 0.0f), camera.getProjection(), camera.getView(), ctx.width, ctx.height);
        screen_pos.y -= 20;
        if (glm::dot(camera.forward(), glm::normalize(e.smooth_transform.position - camera.getPosition())) < 0.2f) {
            continue;
        }

        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::SetNextWindowSizeConstraints({0, 0}, {FLT_MAX, FLT_MAX});
        ImGui::SetNextWindowBgAlpha(0.2f);
        ImGui::SetNextWindowSize({0, 0});
        ImGui::SetNextWindowPos({(float)screen_pos.x, (float)screen_pos.y}, 0, {0.5f, 0.5f});
        ImGui::Begin(("##Name" + std::to_string(idx)).c_str(), nullptr, window_flags);
        ImGui::Text("%s", e.name.c_str());
        ImGui::End();
        ImGui::PopStyleVar();

        idx += 1;
    }
}

void GameView::gui(float dt)
{
    // ImGui::ShowDemoWindow();

    // ImGui::Begin("Shadow map");
    // ImGui::Image((ImTextureID)(intptr_t) world_renderer.shadowmap._depthTexture._texture, ImVec2(ctx.width/3, ctx.height/3), ImVec2(0, 1), ImVec2(1, 0));
    // ImGui::End();

    ImGui::Begin("Debug", nullptr, !_cursor_enabled ? ImGuiWindowFlags_NoInputs : 0);

    ImGui::Text("%s", SimpleProfiler::instance().dump().c_str());

    ImGui::Text("RAM: %.4f / %.4f Go", ((double)getCurrentRSS()) / (1024*1024*1024), ((double)getPeakRSS()) / (1024*1024*1024));

    ImGui::Text("BufferVertices: %d / %d", world_renderer.buffer_allocator_vertices.getFreeSlotsCount(), world_renderer.buffer_allocator_vertices.getMaxSlotsCount());
    ImGui::Text("BufferIndices: %d / %d", world_renderer.buffer_allocator_indices.getFreeSlotsCount(), world_renderer.buffer_allocator_indices.getMaxSlotsCount());

    ImGui::Text("New chunks: %d", (int32_t)Client::instance().new_chunks.size());
    ImGui::Text("ThreadPool{%d} tasks: %d", (int32_t)world_renderer.thread_pool._workers.size(), (int32_t)world_renderer.thread_pool._task_queue.size());

    ImGui::Text("Chunks: %d (%d rendered)", World::instance().getChunkCount(), world_renderer.chunks_drawn);

    ImGui::Text("%.4f secs", dt);
    ImGui::Text("%.2f fps", 1.0f / dt);

    glm::vec3 camera_pos = camera.getPosition();
    ImGui::Text("position: %.2f, %.2f, %.2f", camera_pos.x, camera_pos.y, camera_pos.z);
    ImGui::Text("forward: %.2f, %.2f, %.2f", camera.forward().x, camera.forward().y, camera.forward().z);
    ImGui::Text("block in hand: %d", (int)block_in_hand);

    ImGui::Checkbox("FreeCam", &free_cam);

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
    ImGui::Checkbox("Chunks borders", &_draw_chunks_borders);
    ImGui::Checkbox("Player Chunk borders", &_draw_player_chunk);
    ImGui::Checkbox("Draw player colliders", &_draw_player_colliders);
    ImGui::Checkbox("Delete far chunks", &_delete_far_chunks);
    ImGui::Checkbox("Ambient occlusion", &world_renderer._ambient_occlusion);
    ImGui::SliderFloat("AO strength: ", &world_renderer._ambient_occlusion_strength, 0.0f, 1.0f, "%.2f");

    if (ImGui::Checkbox("VSync", &_vsync)) {
        ctx.setVsync(_vsync);
    }

    ImGui::DragFloat3("Sun direction: ", &world_renderer.sunDir.x, 0.01f, -std::numbers::pi*2, std::numbers::pi*2, "%.2f");
    ImGui::SliderFloat("Shadow Bias: ", &world_renderer.shadowmap._shadow_bias, 0.000001f, 0.001f, "%.6f");
    ImGui::SliderFloat("Shadow Distance: ", &world_renderer._max_shadow_distance, 0.3f, 500.0f, "%.2f");

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
