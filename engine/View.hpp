#pragma once

class Context;

class View {
    public:
        Context& ctx;

    public:
        View(Context& ctx): ctx(ctx) {};
        virtual ~View() = default;

        virtual void onHideView() {}
        virtual void onShowView() {}

        virtual void onUpdate(double time_since_start, float dt) = 0;
        virtual void onDraw(double time_since_start, float dt) = 0;

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

        void onHideView()
        {
        }

        void onShowView()
        {
        }

        void onUpdate(double time_since_start, float dt)
        {
            // physic, logic ...
        }

        void onDraw(double time_since_start, float dt)
        {
            // glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
            // glClear(GL_COLOR_BUFFER_BIT);

            // render
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
            // glViewport(0, 0, width, height);
        }
};
