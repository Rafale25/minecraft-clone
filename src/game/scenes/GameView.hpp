#pragma once

#include "Context.hpp"

#include "View.hpp"
#include "FpsCamera.hpp"

#include "Entity.hpp"
#include "WorldRenderer.hpp"
#include "BlockRaycastHit.hpp"

#include "ChunkMesh.hpp"
#include "DebugDraw.hpp"

class GameView: public View {
public:
    GameView(Context& ctx);

    void onUpdate(double time_since_start, float dt);
    void onHideView();

    void deleteFarChunks();
    void processNewChunks();
    void networkUpdate();

    void playerMovements(float dt);
    void sendTextMessage();
    void placeSphere(const glm::ivec3& center, float radius, BlockType blocktype);
    void setPlayerPosition(const glm::vec3& p);

    void drawPlayersNames();
    void guiWorldEdit();
    void gui(float dt);

    void onDraw(double time_since_start, float dt);
    void onKeyPress(int key);
    void onMousePress(int x, int y, int button);
    void onMouseDrag(int x, int y, int dx, int dy);
    void onMouseScroll(int scroll_x, int scroll_y);
    void onMouseMotion(int x, int y, int dx, int dy);
    void onResize(int width, int height);

private:
    WorldRenderer world_renderer{ctx};

    float network_timer = 1.0f;

    bool _show_debug_gui = false;
    bool _cursor_enabled = false;
    bool _vsync = true;
    bool _delete_far_chunks = true;

    bool _draw_chunks_borders = false;
    bool _draw_player_chunk = false;
    bool _draw_player_colliders = false;
    bool _draw_hit_point = false;

    FPSCamera camera = {
        glm::vec3(0.0f, 25.0, 0.0f), 0.0f, 0.0f,
        60.0f, (float)ctx.width / (float)ctx.height, 0.1f, 2000.0f
    };

    BlockType block_in_hand = BlockType::Grass;
    float bulk_edit_radius = 4.0f;

    BlockRaycastHit player_blockraycasthit;

    // -- Player physic -- //
        bool free_cam = true;
        const float player_height = 1.8f;
        const float player_radius = 0.3f;
        const float player_gravity = 45.0f;
        glm::vec3 player_velocity = {0.0f, 0.0f, 0.0f};
    // --

    // -- World editor -- //
    bool block_selection_mode = false;
    glm::ivec3 blockA = {0.0f, 0.0f, 0.0f};
    glm::ivec3 blockB = {0.0f, 0.0f, 0.0f};
    // --

    char input_text_buffer[4096] = {};
    std::vector<std::string> tchat;
};
