#pragma once

struct GLFWwindow;

#include "View.hpp"

class Context {
    public:
        Context(int width, int height, const char *title, int maximized=0, int samples=4);
        ~Context();

        void run();
        void swapBuffers();

        void imguiNewFrame();
        void imguiRender();
        void imguiInit();

        void showView(View& view);
        void setVsync(bool enable);

        static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
        static void cursor_position_callback(GLFWwindow* window, double x, double y);
        static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
        static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
        static void cursor_enter_callback(GLFWwindow* window, int entered);
        static void framebuffer_size_callback(GLFWwindow* window, int width, int height);

    public:
        GLFWwindow* window = nullptr;
        int keystate[512] = {0}; // GLFW_LAST_KEY iS 348 but I glfw is included only in cpp implementation file (so we take 512 to be safe)
        int width = 0, height = 0;

    private:
        DefaultView m_defaultView{*this};
        View* m_currentView = &m_defaultView;
        int m_mouseX = 0, m_mouseY = 0;
};
