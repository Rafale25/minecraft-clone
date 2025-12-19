#include "GameView.hpp"
#include "GameState.hpp"
#include "World.hpp"
#include "Chunk.hpp"
#include "Entity.hpp"
#include "AABB.hpp"
#include "utils/geometry.hpp"
#include "command_line_args.h"
#include "DebugDraw.hpp"
#include "Profiler.hpp"
#include "Client.hpp" // include before GLFW to avoid macro redefinition warning with MSVC
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <imgui.h>

GameView::GameView(Context& ctx): View(ctx)
{
    glfwSetInputMode(ctx.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (global_argc > 3) {
        GameState::setRenderDistance(std::atoi(global_argv[3]));
    }

    Client::instance().init(tchat, global_argv[1], std::atoi(global_argv[2]));
    Client::instance().Start();


    glGenTextures(4, _texture_view);

    for (int i = 0 ; i < 4 ; ++i) {
        glTextureView(
            _texture_view[i], GL_TEXTURE_2D,
            world_renderer.shadowmap._depthTextureArray, GL_DEPTH_COMPONENT32F,
            0, 1, i, 1
        );

        constexpr GLint rgba[4] = { GL_RED, GL_RED, GL_RED, GL_ONE };
        glTextureParameteriv(_texture_view[i], GL_TEXTURE_SWIZZLE_RGBA, (GLint*)&rgba); // to make the texture grayscale in imgui
    }

}

void GameView::onHideView()
{
    Client::instance().Stop();
    world_renderer.thread_pool.stop();
}

void GameView::onUpdate(double time_since_start, float dt)
{
    legit::Profiler::setEnable(_show_profiler_gui);

    {
        ScopedTask("player");
        playerMovements(dt);
        camera.update(dt);
    }

    {
        ScopedTask("task_queue.execute");
        Client::instance().task_queue.execute();
    }

    {
        ScopedTask("processNewChunks");
        processNewChunks();
    }
    {
        ScopedTask("deleteFarChunks");
        if (_delete_far_chunks) deleteFarChunks();
    }
    {
        ScopedTask("world_renderer.update");
        world_renderer.update();
    }
    {
        ScopedTask("updateEntities");
        World::instance().updateEntities();
    }

    player_blockraycasthit = World::instance().blockRaycast(camera.getPosition(), camera.forward(), 16);

    network_timer -= dt;
    if (network_timer <= 0.0f) {
        network_timer = 1.0f / 20.0f;
        networkUpdate();
    }

    if (player_blockraycasthit.blocktype != BlockType::Air) {
        DebugDraw::instance().drawCube(glm::vec3(player_blockraycasthit.block_pos) + 0.5f, 1.0f, {0.8f, 0.8f, 0.8f});

        if (_draw_hit_point) {
            DebugDraw::instance().drawSphere(player_blockraycasthit.world_pos, 0.05f);
        }
    }

    if (_show_debug_gui) {
        for (const auto& e : World::instance().entities) {
            DebugDraw::instance().drawCuboid(e.transform.position, {0.3f, 1.0f, 0.3f});
        }
    }

    if (_draw_chunks_borders) {
        for (const auto& [pos, chunk] : World::instance().chunks) {
            DebugDraw::instance().drawCube(glm::vec3(pos * CHUNK_SIZE) + glm::vec3(int(CHUNK_SIZE / 2)), CHUNK_SIZE);
        }
    }

    if (_draw_player_chunk) {
        for (int32_t i = 0; i <= CHUNK_SIZE ; ++i) {
            const glm::vec3 py00 = glm::floor(camera.getPosition() / CHUNK_SIZEF) * CHUNK_SIZEF + glm::vec3(0.0f, (float)i, 0.0f);
            const glm::vec3 py11 = glm::floor(camera.getPosition() / CHUNK_SIZEF) * CHUNK_SIZEF + glm::vec3(CHUNK_SIZEF, (float)i, CHUNK_SIZEF);
            const glm::vec3 pz00 = glm::floor(camera.getPosition() / CHUNK_SIZEF) * CHUNK_SIZEF + glm::vec3((float)i, 0.0f, 0.0f);
            const glm::vec3 pz11 = glm::floor(camera.getPosition() / CHUNK_SIZEF) * CHUNK_SIZEF + glm::vec3((float)i, CHUNK_SIZEF, CHUNK_SIZEF);
            const glm::vec3 px00 = glm::floor(camera.getPosition() / CHUNK_SIZEF) * CHUNK_SIZEF + glm::vec3(0.0f, 0.0f, (float)i);
            const glm::vec3 px11 = glm::floor(camera.getPosition() / CHUNK_SIZEF) * CHUNK_SIZEF + glm::vec3(CHUNK_SIZEF, CHUNK_SIZEF, (float)i);

            DebugDraw::instance().drawLine(py00, py00 + glm::vec3{CHUNK_SIZE, 0, 0});
            DebugDraw::instance().drawLine(py00, py00 + glm::vec3{0, 0, CHUNK_SIZE});
            DebugDraw::instance().drawLine(py11, py11 + glm::vec3{-CHUNK_SIZE, 0, 0});
            DebugDraw::instance().drawLine(py11, py11 + glm::vec3{0, 0, -CHUNK_SIZE});

            DebugDraw::instance().drawLine(pz00, pz00 + glm::vec3{0, CHUNK_SIZE, 0});
            DebugDraw::instance().drawLine(pz00, pz00 + glm::vec3{0, 0, CHUNK_SIZE});
            DebugDraw::instance().drawLine(pz11, pz11 + glm::vec3{0, 0, -CHUNK_SIZE});
            DebugDraw::instance().drawLine(pz11, pz11 + glm::vec3{0, -CHUNK_SIZE, 0});

            DebugDraw::instance().drawLine(px00, px00 + glm::vec3{0, CHUNK_SIZE, 0});
            DebugDraw::instance().drawLine(px00, px00 + glm::vec3{CHUNK_SIZE, 0, 0});
            DebugDraw::instance().drawLine(px11, px11 + glm::vec3{-CHUNK_SIZE, 0, 0});
            DebugDraw::instance().drawLine(px11, px11 + glm::vec3{0, -CHUNK_SIZE, 0});
        }

        for (int32_t z = - 1 ; z <= 1 ; ++z) {
        for (int32_t x = - 1 ; x <= 1 ; ++x) {
            const glm::vec3 p = glm::floor(camera.getPosition() / CHUNK_SIZEF) * CHUNK_SIZEF + glm::vec3(x, 0, z) * CHUNK_SIZEF;
            DebugDraw::instance().drawLine(p + glm::vec3(0, -128, 0), p + glm::vec3{0, 128, 0}, {1, 0, 1});
            DebugDraw::instance().drawLine(p + glm::vec3(CHUNK_SIZE, -128, 0), p + glm::vec3{CHUNK_SIZE, 128, 0}, {1, 0, 1});
            DebugDraw::instance().drawLine(p + glm::vec3(0, -128, CHUNK_SIZE), p + glm::vec3{0, 128, CHUNK_SIZE}, {1, 0, 1});
            DebugDraw::instance().drawLine(p + glm::vec3(CHUNK_SIZE, -128, CHUNK_SIZE), p + glm::vec3{CHUNK_SIZE, 128, CHUNK_SIZE}, {1, 0, 1});
        }
        }
    }

    if (block_selection_mode) {
        const glm::ivec3 min = glm::min(blockA, blockB);
        const glm::ivec3 max = glm::max(blockA, blockB);
        DebugDraw::instance().drawCuboidMinMax(min, max + 1);
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
            ctx.keystate[GLFW_KEY_LEFT_SHIFT] == GLFW_PRESS ? 220.0f : 10.0f
        );

        player_velocity = {0.0f, 0.0f, 0.0f};

        if (!_cursor_enabled && !ImGui::GetIO().WantCaptureKeyboard) camera.move(delta);
        return;
    }

    delta *= 0.5f;

    glm::vec3 forward_xz = glm::normalize(glm::vec3(camera.forward().x, 0.0f, camera.forward().z));
    glm::vec3 move_vector = -delta.x * camera.right() + delta.z * forward_xz;

    glm::vec3 player_feet_position = camera.getPosition() - glm::vec3(0.0f, player_height, 0.0f);

    // printf("Y: %.6f %.6f %.6f\n", glm::mod(player_feet_position.x, 1.0f), glm::mod(player_feet_position.y, 1.0f), glm::mod(player_feet_position.z, 1.0f));

    AABB player_aabb_under_feet = {
        player_feet_position + glm::vec3(-player_radius+0.01f, -0.001f, -player_radius+0.01f),
        player_feet_position + glm::vec3(player_radius-0.01f, 0.001f, player_radius-0.01f)
    };

    AABB player_aabb = {
        player_feet_position + glm::vec3(-0.3f, 0.0f, -0.3f),
        player_feet_position + glm::vec3(0.3f, player_height, 0.3f)};

    if (_draw_player_colliders) DebugDraw::instance().drawCuboidMinMax(player_aabb_under_feet.min, player_aabb_under_feet.max, {0.4f, 0.2, 0.8});

    std::vector<AABB> neighbours_blocks_AABB;
    for (int32_t z = -3 ; z <= 3 ; ++z) {
    for (int32_t y = -3 ; y <= 3 ; ++y) {
    for (int32_t x = -3 ; x <= 3 ; ++x) {
        glm::vec3 p = glm::floor(player_feet_position + glm::vec3(x, y, z));
        if (World::instance().getBlock(p) != BlockType::Air) {
            const AABB block_aabb = {p, p + 1.0f};
            if (AABB::AABBtoAABB(player_aabb, block_aabb)) continue;  // Don't create collider if player already inside block

            neighbours_blocks_AABB.push_back(block_aabb);

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
    if (is_grounded && ctx.keystate[GLFW_KEY_SPACE]) { // JUMP
        player_velocity.y += 12.0f;
    }

    player_velocity.y -= player_gravity * dt;
    player_velocity.y = glm::clamp(player_velocity.y, -400.0f, 400.0f);

    if (is_grounded) {
        player_velocity += move_vector * 1.0f;
    } else {
        player_velocity += move_vector * 0.1f;
    }

    glm::vec3 next_pos = player_feet_position + player_velocity * dt;


    // -- PLAYER/ENTITY AABB COLLISION -- //

    AABB player_aabb_y = {
        glm::vec3(player_feet_position.x, next_pos.y, player_feet_position.z) + glm::vec3(-0.3f, 0.0f, -0.3f),
        glm::vec3(player_feet_position.x, next_pos.y, player_feet_position.z) + glm::vec3(0.3f, player_height, 0.3f)};
    if (_draw_player_colliders) DebugDraw::instance().drawCuboidMinMax(player_aabb_y.min, player_aabb_y.max, {1.0f, 0.2, 0.8});

    // Y
    for (const auto& aabb : neighbours_blocks_AABB) {
        if (AABB::AABBtoAABB(aabb, player_aabb_y)) {
            float dy = AABB::AABBtoAABBOverlapDistance(aabb, player_aabb_y).y;
            next_pos.y += dy + glm::sign(dy) * 0.0001f;
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
            next_pos.x += dx + glm::sign(dx) * 0.0001f;
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
            next_pos.z += dz + glm::sign(dz) * 0.0001f;
            player_velocity.z = 0.0f;
            break;
        }
    }
    // ------------------------------------------------- //


    camera.setPosition(next_pos + glm::vec3(0.0f, player_height, 0.0f));
}

void GameView::deleteFarChunks() // TODO: only do this when moving between chunks
{
    const std::lock_guard<std::shared_mutex> lock(World::instance().chunks_mutex);

    std::vector<glm::ivec3> pos_to_delete;

    auto& world_chunks = World::instance().chunks;
    for (const auto& [pos, chunk] : world_chunks) {

        bool is_in_view_distance = isInManhattanDistance(
                                    World::worldToChunkCoord(camera.getPosition()),
                                    chunk->pos,
                                    GameState::getRenderDistance() + world_renderer.CHUNK_DELETE_DISTANCE_OFFSET);
        if (!is_in_view_distance) {
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
        // bool is_in_view_distance = isInManhattanDistance(
        //                             camera.getPosition(),
        //                             glm::vec3(chunk_data->pos) * 16.0f,
        //                             GameState::getRenderDistance() * 16.0f + world_renderer.CHUNK_DELETE_DISTANCE_OFFSET);

        // if (!is_in_view_distance) {
        //     delete chunk_data;
        //     continue;
        // }

        const Chunk* chunk = World::instance().setChunk(chunk_data->pos, chunk_data->blocks);
        if (chunk) {
            world_renderer.onAddedChunk(chunk_data->pos);
        }
        delete chunk_data;
    }

}

void GameView::networkUpdate()
{
    Client::instance().sendUpdateEntityPacket(camera.getPosition(), camera.getYaw() + glm::pi<float>(), - camera.getPitch());
}

void GameView::onDraw(double time_since_start, float dt)
{
    {
        // ScopedTask("world_renderer.render");
        world_renderer.render(camera);
    }

    if (_show_debug_gui) {
        gui(dt);
    }

    drawPlayersNames();
}

void GameView::sendTextMessage() {
    if (strlen(input_text_buffer) <= 0) return;
    Client::instance().sendChatMessagePacket(input_text_buffer);
    memset(input_text_buffer, 0, sizeof(input_text_buffer));
}

void GameView::placeSphere(const glm::ivec3& center, float radius, BlockType blocktype)
{
    std::vector<glm::ivec3> positions;

    int32_t iradius = int32_t(radius);
    for (int32_t x = -iradius ; x <= iradius ; ++x) {
    for (int32_t y = -iradius ; y <= iradius ; ++y) {
    for (int32_t z = -iradius ; z <= iradius ; ++z) {
        glm::ivec3 p = center + glm::ivec3{x, y, z};
        if (glm::distance2(glm::vec3(center), glm::vec3(p)) > radius*radius) continue;
        positions.push_back(p);
    }
    }
    }
    Client::instance().sendBlockBulkEditPacketMonotype(positions, blocktype);
}

void GameView::setPlayerPosition(const glm::vec3& p) {
    camera.setPosition(p);
}

void GameView::onKeyPress(int key)
{
    if (key == GLFW_KEY_C && !ImGui::GetIO().WantCaptureKeyboard) {
        _cursor_enabled = !_cursor_enabled;

        if (_cursor_enabled)
            glfwSetInputMode(ctx.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        else
            glfwSetInputMode(ctx.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    // toggle flight when double pressing space
    if (key == GLFW_KEY_SPACE) {
        double t = glfwGetTime();
        if (t - last_jump_press < 0.25f) {
            free_cam = !free_cam;
        }
        last_jump_press = t;
    }

    if (key == GLFW_KEY_R) {
        for (auto& [_, program]: world_renderer._shaders) {
            program.reload();
        }
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

    // Pick block
    if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
        BlockType block = World::instance().blockRaycast(camera.getPosition(), camera.forward(), 64).blocktype;
        block_in_hand = block;
    }

    if (block_selection_mode) {
        if (!player_blockraycasthit.hit) return;

        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            blockA = player_blockraycasthit.block_pos;
        } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
            blockB = player_blockraycasthit.block_pos;
        }

        return;
    }

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (ctx.keystate[GLFW_KEY_LEFT_ALT])
            placeSphere(player_blockraycasthit.block_pos, bulk_edit_radius, BlockType::Air);
        else
            if (player_blockraycasthit.blocktype != BlockType::Air) {
                Client::instance().sendBreakBlockPacket(player_blockraycasthit.block_pos);
            }
    } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (ctx.keystate[GLFW_KEY_LEFT_ALT])
            placeSphere(player_blockraycasthit.block_pos, bulk_edit_radius, block_in_hand);
        else {
            if (player_blockraycasthit.blocktype != BlockType::Air) {
                Client::instance().sendPlaceBlockPacket(player_blockraycasthit.block_pos + glm::ivec3(player_blockraycasthit.normal), block_in_hand);
            }
        }
    }
}

void GameView::onMouseDrag(int x, int y, int dx, int dy)
{
    if (_show_debug_gui && ImGui::GetIO().WantCaptureMouse) return;
}

void GameView::onMouseScroll(int scroll_x, int scroll_y)
{
    int32_t block = ((int32_t)block_in_hand + scroll_y) % ((int32_t)BlockType::INVALID-1);
    if (block < 1)
        block += (int32_t)BlockType::INVALID-1;
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



// // #Sort Chunks
// NOTE: the chunks are sent to be queued before having the chance to be sorted by distance (the solution is to sort the chunks on the server)
// const glm::vec3 camPos = camera.getPosition();
// std::sort(Client::instance().new_chunks.begin(), Client::instance().new_chunks.end(),
//     [camPos](const Packet::Server::ChunkPacket* l, const Packet::Server::ChunkPacket* r)
//     {
//         return glm::distance2(camPos, glm::vec3(l->pos*16)) > glm::distance2(camPos, glm::vec3(r->pos*16));
//     });
