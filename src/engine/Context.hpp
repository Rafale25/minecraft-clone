#pragma once

#include <iostream>

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "View.hpp"

class Context {
    public:
        Context(int width, int height, const char *title);
        ~Context();

        void run();
        void swapBuffers();

        void imguiNewFrame();
        void imguiRender();
        void imguiInit();

        void show_view(View* view);
        void setVsync(int value);

        static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
        static void cursor_position_callback(GLFWwindow* window, double x, double y);
        static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
        static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
        static void cursor_enter_callback(GLFWwindow* window, int entered);
        static void framebuffer_size_callback(GLFWwindow* window, int width, int height);

    public:
        GLFWwindow* window;

    private:
        View* _current_view;
        int _mouseX, _mouseY;
};
