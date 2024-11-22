#pragma once

#include "Context.hpp"

#include "View.hpp"
#include "FpsCamera.hpp"

#include "Entity.hpp"
#include "WorldRenderer.hpp"
#include "BlockRaycastHit.hpp"
#include "ThreadPool.h"

class GameView: public View {
    public:
        GameView(Context& ctx);

        void onUpdate(double time_since_start, float dt);
        void onHideView();

        void update3x3Chunks(const glm::ivec3& chunk_pos);

        void consumeNewChunks();
        void consumeTaskQueue();
        void networkUpdate();

        void sendTextMessage();
        void placeSphere(const glm::ivec3& center, float radius, BlockType blocktype);
        void setPlayerPosition(const glm::vec3& p);

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

        bool _cursor_enabled = false;
        bool _show_debug_gui = false;
        bool _vsync = true;

        FPSCamera camera = {
            glm::vec3(10.0f, 25.0, 12.0f), 0.0f, 0.0f,
            60.0f, (float)ctx.width / (float)ctx.height, 0.1f, 1000.0f
        };

        BlockType block_in_hand = BlockType::Grass;
        float bulk_edit_radius = 4.0f;

        BlockRaycastHit player_blockraycasthit;

        char input_text_buffer[4096] = {0};
        std::vector<std::string> tchat;

        ThreadPool thread_pool{1};
        TaskQueue main_task_queue;
};
