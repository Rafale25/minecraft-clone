#pragma once

#include "View.hpp"
#include "Camera.hpp"

class GameView: public View {
    public:
        GameView(Context& ctx): View(ctx) {}

        void onUpdate(float time_since_start, float dt)
        {
        }

        void onDraw(float time_since_start, float dt)
        {
            glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            gui(dt);
        }

        void gui(float dt)
        {
            ctx.imguiNewFrame();
            ImGui::Begin("Debug");

            ImGui::Text("%.4f ms", dt);
            ImGui::Text("%.2f fps", 1.0f / dt);

            ImGui::End();
            ctx.imguiRender();
        }

        void onKeyPress(int key)
        {
        }

        void onKeyRelease(int key)
        {
        }

        void onMouseMotion(int x, int y, int dx, int dy)
        {
        }

        void onMouseDrag(int x, int y, int dx, int dy)
        {
        }

        void onMousePress(int x, int y, int button)
        {
        }

        void onMouseRelease(int x, int y, int button)
        {
        }

        void onMouseScroll(int scroll_x, int scroll_y)
        {
        }

        void onResize(int width, int height)
        {
            glViewport(0, 0, width, height);
        }
};
