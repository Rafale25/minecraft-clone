#pragma once

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "view.hpp"

// TODO: separate view from context (make a ViewManager)

class Context {
    public:
        Context(int width, int height, const char *title, int maximized=GL_FALSE, int samples=4);
        ~Context();

        void run();
        void swapBuffers();

        void imguiNewFrame();
        void imguiRender();
        void imguiInit();

        void showView(View& view);
        void setVsync(int value);

        static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
        static void cursor_position_callback(GLFWwindow* window, double x, double y);
        static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
        static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
        static void cursor_enter_callback(GLFWwindow* window, int entered);
        static void framebuffer_size_callback(GLFWwindow* window, int width, int height);

    public:
        GLFWwindow* window;
        int keyState[GLFW_KEY_LAST] = {0};
        int width, height;

    private:
        DefaultView _default_view{*this};
        View* _current_view = &_default_view;
        int _mouseX, _mouseY;
};
