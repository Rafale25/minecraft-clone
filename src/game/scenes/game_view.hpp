#pragma once

#include "context.hpp"

#include "view.hpp"
#include "fps_camera.hpp"
#include "program.h"

#include "entity.hpp"

#include "BlockRaycastHit.hpp"

#include "geometry.hpp"
#include "shadow_map.hpp"

#include "thread_pool.hpp"

class GameView: public View {
    public:
        GameView(Context& ctx);

        void onUpdate(double time_since_start, float dt);

        void consumeNewChunks();
        void consumeTaskQueue();
        void networkUpdate();

        void onDraw(double time_since_start, float dt);
        void render_world(const Program &shader, bool use_frustum_culling=true);
        void gui(float dt);

        void sendTextMessage();
        void placeSphere(const glm::ivec3& center, float radius, BlockType blocktype);
        void setPlayerPosition(const glm::vec3& p);
        void onKeyPress(int key);
        void onMousePress(int x, int y, int button);
        void onMouseDrag(int x, int y, int dx, int dy);
        void onMouseScroll(int scroll_x, int scroll_y);
        void onMouseMotion(int x, int y, int dx, int dy);
        void onResize(int width, int height);

    private:
        Program cube_shader{"./assets/shaders/cube.vs", "./assets/shaders/cube.fs"};
        Program cube_shadowmapping_shader{"./assets/shaders/cube_shadowmap.vs", "./assets/shaders/cube_shadowmap.fs"};
        Program mesh_shader{"./assets/shaders/mesh.vs", "./assets/shaders/mesh.fs"};
        Program skybox_shader{"./assets/shaders/skybox.vs", "./assets/shaders/skybox.fs"};

        Shadowmap shadowmap{ctx, 4096, 4096};
        glm::vec3 sunDir = glm::normalize(glm::vec3(20.0f, 50.0f, 20.0f));
        Mesh skybox_quad = Geometry::quad_2d();

        GLuint ssbo_texture_handles;

        float network_timer = 1.0f;

        bool _cursor_enabled = false;
        bool _show_debug_gui = false;
        bool _wireframe = false;
        bool _vsync = true;

        int _chunks_drawn;

        FPSCamera camera = {
            glm::vec3(10.0f, 25.0, 12.0f), 0.0f, 0.0f,
            60.0f, (float)ctx.width / (float)ctx.height, 0.1f, 1000.0f
        };

        BlockType blockInHand = BlockType::Grass;
        float bulkEditRadius = 4.0f;

        BlockRaycastHit player_blockraycasthit;

        char input_text_buffer[4096] = {0};
        std::vector<std::string> tchat;

        ThreadPool thread_pool{6};
        TaskQueue main_task_queue;

        /*
        class WorldRenderer {
            void setRenderState()

            void renderWorld(Camera)
            void renderWorldDepth(Camera)

            void renderEntities(Camera)
            void renderEntitiesDepth(Camera)

            void renderSkybox(Camera)

            void renderShadowmap(Camera)

            GLuint ssbo_texture_handles

            uint _chunks_drawn
        }
        */
};
