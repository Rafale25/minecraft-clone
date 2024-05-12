#pragma once

#include <GLFW/glfw3.h>

class Context;

class View {
    public:
        Context& ctx;

    public:
        View(Context& ctx): ctx(ctx) {};

        virtual void onHideView() {}
        virtual void onShowView() {}

        virtual void onUpdate(float time_since_start, float dt) = 0;
        virtual void onDraw(float time_since_start, float dt) = 0;

        virtual void onKeyPress(int key) {}
        virtual void onKeyRelease(int key) {}

        virtual void onMouseMotion(int x, int y, int dx, int dy) {}
        virtual void onMouseDrag(int x, int y, int dx, int dy) {}
        virtual void onMouseEnter(int x, int y) {}
        virtual void onMouseLeave(int x, int y) {}

        virtual void onMousePress(int x, int y, int button) {}
        virtual void onMouseRelease(int x, int y, int button) {}
        virtual void onMouseScroll(int scroll_x, int scroll_y) {}
        virtual void onResize(int width, int height) {}
};

class DefaultView: public View {
    public:
        DefaultView(Context& ctx): View(ctx) {}

        void onUpdate(float time_since_start, float dt)
        {
            // physic, logic ...
        }

        void onDraw(float time_since_start, float dt)
        {
            glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            // render
        }

        void onKeyPress(int key)
        {
            // std::cout << key << std::endl;
        }

        void onKeyRelease(int key)
        {
            // std::cout << key << std::endl;
        }

        void onMouseMotion(int x, int y, int dx, int dy)
        {
            // printf("%d %d %d %d\n", x, y, dx, dy);
        }

        void onMouseDrag(int x, int y, int dx, int dy)
        {
            // printf("%d %d %d %d\n", x, y, dx, dy);
        }

        void onMousePress(int x, int y, int button)
        {
            // printf("%d %d %d\n", x, y, button);
        }

        void onMouseRelease(int x, int y, int button)
        {
            // printf("%d %d %d\n", x, y, button);
        }

        void onMouseScroll(int scroll_x, int scroll_y)
        {
            // printf("%d %d\n", scroll_x, scroll_y);
        }

        void onResize(int width, int height)
        {
            glViewport(0, 0, width, height);
        }
};
