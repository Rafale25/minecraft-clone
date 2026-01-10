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
#include <glm/gtc/random.hpp>

GameView::GameView(Context& ctx): View(ctx)
{
    glfwSetInputMode(ctx.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (global_argc > 3) {
        GameState::setRenderDistance(std::atoi(global_argv[3]));
    }

    Client::instance().init(m_tchat, global_argv[1], std::atoi(global_argv[2]));
    Client::instance().Start();

    m_scriptManager.registerScript(SCRIPTS_PATH "gun.lua");
    m_scriptManager.refresh();

    m_scriptManager.init(m_camera, *this);
}

void GameView::onHideView()
{
    Client::instance().Stop();
    m_worldRenderer.m_threadPool.stop();
}

void GameView::onUpdate(double time_since_start, float dt)
{
    legit::Profiler::setEnable(m_showProfilerGui);

    m_scriptManager.update(time_since_start, dt);

    {
        ScopedTask("player");
        playerMovements(dt);
        m_camera.update(dt);
    }

    {
        ScopedTask("task_queue.execute");
        Client::instance().m_taskQueue.execute();
    }

    {
        ScopedTask("processNewChunks");
        processNewChunks();
    }
    {
        ScopedTask("deleteFarChunks");
        if (m_deleteFarChunks) deleteFarChunks();
    }
    {
        ScopedTask("world_renderer.update");
        m_worldRenderer.update();
    }
    {
        ScopedTask("updateEntities");
        World::instance().updateEntities();
    }

    m_playerBlockRaycastHit = World::instance().blockRaycast(m_camera.getPosition(), m_camera.forward(), 16);

    m_networkTimer -= dt;
    if (m_networkTimer <= 0.0f) {
        m_networkTimer = 1.0f / 20.0f;
        networkUpdate();
    }

    if (m_playerBlockRaycastHit.blocktype != BlockType::Air) {
        DebugDraw::instance().drawCube(glm::vec3(m_playerBlockRaycastHit.block_pos) + 0.5f, 1.0f, {0.8f, 0.8f, 0.8f});

        if (m_drawHitPoint) {
            DebugDraw::instance().drawSphere(m_playerBlockRaycastHit.world_pos, 0.05f);
        }
    }

    if (m_showDebugGui) {
        for (const auto& e : World::instance().m_entities) {
            DebugDraw::instance().drawCuboid(e.transform.position, {0.3f, 1.0f, 0.3f});
        }
    }

    if (m_drawChunksBorders) {
        for (const auto& [pos, chunk] : World::instance().m_chunks) {
            DebugDraw::instance().drawCube(glm::vec3(pos * CHUNK_SIZE) + glm::vec3(int(CHUNK_SIZE / 2)), CHUNK_SIZE);
        }
    }

    if (m_drawPlayerChunk) {
        for (int32_t i = 0; i <= CHUNK_SIZE ; ++i) {
            const glm::vec3 py00 = glm::floor(m_camera.getPosition() / CHUNK_SIZEF) * CHUNK_SIZEF + glm::vec3(0.0f, (float)i, 0.0f);
            const glm::vec3 py11 = glm::floor(m_camera.getPosition() / CHUNK_SIZEF) * CHUNK_SIZEF + glm::vec3(CHUNK_SIZEF, (float)i, CHUNK_SIZEF);
            const glm::vec3 pz00 = glm::floor(m_camera.getPosition() / CHUNK_SIZEF) * CHUNK_SIZEF + glm::vec3((float)i, 0.0f, 0.0f);
            const glm::vec3 pz11 = glm::floor(m_camera.getPosition() / CHUNK_SIZEF) * CHUNK_SIZEF + glm::vec3((float)i, CHUNK_SIZEF, CHUNK_SIZEF);
            const glm::vec3 px00 = glm::floor(m_camera.getPosition() / CHUNK_SIZEF) * CHUNK_SIZEF + glm::vec3(0.0f, 0.0f, (float)i);
            const glm::vec3 px11 = glm::floor(m_camera.getPosition() / CHUNK_SIZEF) * CHUNK_SIZEF + glm::vec3(CHUNK_SIZEF, CHUNK_SIZEF, (float)i);

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
            const glm::vec3 p = glm::floor(m_camera.getPosition() / CHUNK_SIZEF) * CHUNK_SIZEF + glm::vec3(x, 0, z) * CHUNK_SIZEF;
            DebugDraw::instance().drawLine(p + glm::vec3(0, -128, 0), p + glm::vec3{0, 128, 0}, {1, 0, 1});
            DebugDraw::instance().drawLine(p + glm::vec3(CHUNK_SIZE, -128, 0), p + glm::vec3{CHUNK_SIZE, 128, 0}, {1, 0, 1});
            DebugDraw::instance().drawLine(p + glm::vec3(0, -128, CHUNK_SIZE), p + glm::vec3{0, 128, CHUNK_SIZE}, {1, 0, 1});
            DebugDraw::instance().drawLine(p + glm::vec3(CHUNK_SIZE, -128, CHUNK_SIZE), p + glm::vec3{CHUNK_SIZE, 128, CHUNK_SIZE}, {1, 0, 1});
        }
        }
    }

    if (m_blockSelectionMode) {
        const glm::ivec3 min = glm::min(m_blockA, m_blockB);
        const glm::ivec3 max = glm::max(m_blockA, m_blockB);
        DebugDraw::instance().drawCuboidMinMax(min, max + 1);
    }
}

void GameView::playerMovements(float dt)
{
    if (m_cursorEnabled || ImGui::GetIO().WantCaptureKeyboard) return;

    glm::vec3 delta = {
        ctx.keystate[GLFW_KEY_A] - ctx.keystate[GLFW_KEY_D],
        ctx.keystate[GLFW_KEY_LEFT_CONTROL] - ctx.keystate[GLFW_KEY_SPACE],
        ctx.keystate[GLFW_KEY_W] - ctx.keystate[GLFW_KEY_S]
    };

    if (m_freeCamEnabled) {
        m_camera.setSpeed(
            ctx.keystate[GLFW_KEY_LEFT_SHIFT] == GLFW_PRESS ? 220.0f : 10.0f
        );

        m_playerVelocity = {0.0f, 0.0f, 0.0f};

        if (!m_cursorEnabled && !ImGui::GetIO().WantCaptureKeyboard) m_camera.move(delta);
        return;
    }

    delta *= 0.5f;

    glm::vec3 forward_xz = glm::normalize(glm::vec3(m_camera.forward().x, 0.0f, m_camera.forward().z));
    glm::vec3 move_vector = -delta.x * m_camera.right() + delta.z * forward_xz;

    glm::vec3 player_feet_position = m_camera.getPosition() - glm::vec3(0.0f, m_playerHeight, 0.0f);

    // printf("Y: %.6f %.6f %.6f\n", glm::mod(player_feet_position.x, 1.0f), glm::mod(player_feet_position.y, 1.0f), glm::mod(player_feet_position.z, 1.0f));

    AABB player_aabb_under_feet = {
        player_feet_position + glm::vec3(-m_playerRadius+0.01f, -0.001f, -m_playerRadius+0.01f),
        player_feet_position + glm::vec3(m_playerRadius-0.01f, 0.001f, m_playerRadius-0.01f)
    };

    AABB player_aabb = {
        player_feet_position + glm::vec3(-0.3f, 0.0f, -0.3f),
        player_feet_position + glm::vec3(0.3f, m_playerHeight, 0.3f)};

    if (m_drawPlayerColliders) DebugDraw::instance().drawCuboidMinMax(player_aabb_under_feet.min, player_aabb_under_feet.max, {0.4f, 0.2, 0.8});

    std::vector<AABB> neighbours_blocks_AABB;
    for (int32_t z = -3 ; z <= 3 ; ++z) {
    for (int32_t y = -3 ; y <= 3 ; ++y) {
    for (int32_t x = -3 ; x <= 3 ; ++x) {
        glm::vec3 p = glm::floor(player_feet_position + glm::vec3(x, y, z));
        if (World::instance().getBlock(p) != BlockType::Air) {
            const AABB block_aabb = {p, p + 1.0f};
            if (AABB::AABBtoAABB(player_aabb, block_aabb)) continue;  // Don't create collider if player already inside block

            neighbours_blocks_AABB.push_back(block_aabb);

            if (m_drawPlayerColliders) DebugDraw::instance().drawCuboidMinMax(p, p + 1.0f);
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
        m_playerVelocity.x *= 0.9f;
        m_playerVelocity.z *= 0.9f;
    }
    if (is_grounded && ctx.keystate[GLFW_KEY_SPACE]) { // JUMP
        m_playerVelocity.y += 12.0f;
    }

    m_playerVelocity.y -= m_playerGravity * dt;
    m_playerVelocity.y = glm::clamp(m_playerVelocity.y, -400.0f, 400.0f);

    if (is_grounded) {
        m_playerVelocity += move_vector * 1.0f;
    } else {
        m_playerVelocity += move_vector * 0.1f;
    }

    glm::vec3 next_pos = player_feet_position + m_playerVelocity * dt;


    // -- PLAYER/ENTITY AABB COLLISION -- //
    {
        AABB player_aabb_y = {
            glm::vec3(player_feet_position.x, next_pos.y, player_feet_position.z) + glm::vec3(-0.3f, 0.0f, -0.3f),
            glm::vec3(player_feet_position.x, next_pos.y, player_feet_position.z) + glm::vec3(0.3f, m_playerHeight, 0.3f)};
        if (m_drawPlayerColliders) DebugDraw::instance().drawCuboidMinMax(player_aabb_y.min, player_aabb_y.max, {1.0f, 0.2, 0.8});

        // Y
        for (const auto& aabb : neighbours_blocks_AABB) {
            if (AABB::AABBtoAABB(aabb, player_aabb_y)) {
                float dy = AABB::AABBtoAABBOverlapDistance(aabb, player_aabb_y).y;
                next_pos.y += dy + glm::sign(dy) * 0.0001f;
                m_playerVelocity.y = 0.0f;
                break;
            }
        }

        AABB player_aabb_x = {
            glm::vec3(next_pos.x, next_pos.y, player_feet_position.z) + glm::vec3(-0.3f, 0.0f, -0.3f),
            glm::vec3(next_pos.x, next_pos.y, player_feet_position.z) + glm::vec3(0.3f, m_playerHeight, 0.3f)};

        // X
        for (const auto& aabb : neighbours_blocks_AABB) {
            if (AABB::AABBtoAABB(aabb, player_aabb_x)) {
                float dx = AABB::AABBtoAABBOverlapDistance(aabb, player_aabb_x).x;
                next_pos.x += dx + glm::sign(dx) * 0.0001f;
                m_playerVelocity.x = 0.0f;
                break;
            }
        }

        AABB player_aabb_z = {
            glm::vec3(next_pos.x, next_pos.y, next_pos.z) + glm::vec3(-0.3f, 0.0f, -0.3f),
            glm::vec3(next_pos.x, next_pos.y, next_pos.z) + glm::vec3(0.3f, m_playerHeight, 0.3f)};

        // Z
        for (const auto& aabb : neighbours_blocks_AABB) {
            if (AABB::AABBtoAABB(aabb, player_aabb_z)) {
                float dz = AABB::AABBtoAABBOverlapDistance(aabb, player_aabb_z).z;
                next_pos.z += dz + glm::sign(dz) * 0.0001f;
                m_playerVelocity.z = 0.0f;
                break;
            }
        }
    }


    m_camera.setPosition(next_pos + glm::vec3(0.0f, m_playerHeight, 0.0f));
}

void GameView::deleteFarChunks() // TODO: only do this when moving between chunks
{
    const std::lock_guard<std::shared_mutex> lock(World::instance().m_chunksMutex);

    std::vector<glm::ivec3> pos_to_delete;

    auto& world_chunks = World::instance().m_chunks;
    for (const auto& [pos, chunk] : world_chunks) {

        bool is_in_view_distance = isInManhattanDistance(
                                    World::worldToChunkCoord(m_camera.getPosition()),
                                    chunk->pos,
                                    GameState::getRenderDistance() + m_worldRenderer.CHUNK_DELETE_DISTANCE_OFFSET);
        if (!is_in_view_distance) {
            pos_to_delete.push_back(pos);
        }
    }

    for (const auto &pos : pos_to_delete) {
        World::instance().deleteChunk(pos);
        m_worldRenderer.onDeletedChunk(pos);
    }
}

void GameView::processNewChunks()
{
    const std::lock_guard<std::mutex> lock(Client::instance().m_newChunksMutex);

    while (Client::instance().m_newChunks.size() > 0) {

        Packet::Server::ChunkPacket* chunk_data = Client::instance().m_newChunks.back();
        Client::instance().m_newChunks.pop_back();

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
            m_worldRenderer.onAddedChunk(chunk_data->pos);
        }
        delete chunk_data;
    }

}

void GameView::networkUpdate()
{
    Client::instance().sendUpdateEntityPacket(m_camera.getPosition(), m_camera.getYaw() + glm::pi<float>(), - m_camera.getPitch());
}

void GameView::onDraw(double time_since_start, float dt)
{
    {
        // ScopedTask("world_renderer.render");
        m_worldRenderer.render(m_camera);
    }

    if (m_showDebugGui) {
        gui(dt);
    }

    drawPlayersNames();
}

void GameView::sendTextMessage() {
    if (strlen(m_inputTextBuffer) <= 0) return;
    Client::instance().sendChatMessagePacket(m_inputTextBuffer);
    memset(m_inputTextBuffer, 0, sizeof(m_inputTextBuffer));
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
    }}}
    Client::instance().sendBlockBulkEditPacketMonotype(positions, blocktype);
}

void GameView::setPlayerPosition(const glm::vec3& p) {
    m_camera.setPosition(p);
}

void GameView::onKeyPress(int key)
{
    if (key == GLFW_KEY_C && !ImGui::GetIO().WantCaptureKeyboard) {
        m_cursorEnabled = !m_cursorEnabled;

        if (m_cursorEnabled)
            glfwSetInputMode(ctx.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        else
            glfwSetInputMode(ctx.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    // toggle flight when double pressing space
    if (key == GLFW_KEY_SPACE) {
        double t = glfwGetTime();
        if (t - m_lastJumpPress < 0.25f) {
            m_freeCamEnabled = !m_freeCamEnabled;
        }
        m_lastJumpPress = t;
    }

    if (key == GLFW_KEY_R) {
        for (auto& [_, program]: m_worldRenderer._shaders) {
            program.reload();
        }
    }

    if (key == GLFW_KEY_F11) {
        // GLFWmonitor* monitor = glfwGetWindowMonitor(ctx.window);
        // GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        // const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        // glfwSetWindowMonitor(ctx.window, monitor, 0, 0, mode->width, mode->height, 0);
    }

    m_scriptManager.onKeyPress(key);

    if (!ImGui::GetIO().WantCaptureKeyboard) {
        if (key == GLFW_KEY_P) {
            m_showDebugGui = !m_showDebugGui;
        }
    }
}

void GameView::onMousePress(int x, int y, int button) {
    if (m_showDebugGui && ImGui::GetIO().WantCaptureMouse) return;

    // Pick block
    if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
        BlockType block = World::instance().blockRaycast(m_camera.getPosition(), m_camera.forward(), 64).blocktype;
        m_blockInHand = block;
    }

    if (m_blockSelectionMode) {
        if (!m_playerBlockRaycastHit.hit) return;

        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            m_blockA = m_playerBlockRaycastHit.block_pos;
        } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
            m_blockB = m_playerBlockRaycastHit.block_pos;
        }

        return;
    }

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (ctx.keystate[GLFW_KEY_LEFT_ALT])
            placeSphere(m_playerBlockRaycastHit.block_pos, m_bulkEditRadius, BlockType::Air);
        else
            if (m_playerBlockRaycastHit.blocktype != BlockType::Air) {
                Client::instance().sendBreakBlockPacket(m_playerBlockRaycastHit.block_pos);
            }
    } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (ctx.keystate[GLFW_KEY_LEFT_ALT])
            placeSphere(m_playerBlockRaycastHit.block_pos, m_bulkEditRadius, m_blockInHand);
        else {
            if (m_playerBlockRaycastHit.blocktype != BlockType::Air) {
                Client::instance().sendPlaceBlockPacket(m_playerBlockRaycastHit.block_pos + glm::ivec3(m_playerBlockRaycastHit.normal), m_blockInHand);
            }
        }
    }
}

void GameView::onMouseDrag(int x, int y, int dx, int dy)
{
    if (m_showDebugGui && ImGui::GetIO().WantCaptureMouse) return;
}

void GameView::onMouseScroll(int scroll_x, int scroll_y)
{
    int32_t block = ((int32_t)m_blockInHand + scroll_y) % ((int32_t)BlockType::INVALID-1);
    if (block < 1)
        block += (int32_t)BlockType::INVALID-1;
    m_blockInHand = (BlockType)block;
}

void GameView::onMouseMotion(int x, int y, int dx, int dy)
{
    if (!m_cursorEnabled)
        m_camera.onMouseMotion(x, y, dx, dy);
}

void GameView::onResize(int width, int height)
{
    glViewport(0, 0, width, height);
    m_camera.aspectRatio = (float)width / (float)height;

    m_worldRenderer.onResize(width, height);
}



// // #Sort Chunks
// NOTE: the chunks are sent to be queued before having the chance to be sorted by distance (the solution is to sort the chunks on the server)
// const glm::vec3 camPos = camera.getPosition();
// std::sort(Client::instance().new_chunks.begin(), Client::instance().new_chunks.end(),
//     [camPos](const Packet::Server::ChunkPacket* l, const Packet::Server::ChunkPacket* r)
//     {
//         return glm::distance2(camPos, glm::vec3(l->pos*16)) > glm::distance2(camPos, glm::vec3(r->pos*16));
//     });
