#include <iostream>
#include "Context.hpp"
#include "View.hpp"

// #include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

static void GLFW_error(int error, const char* description)
{
    fprintf(stderr, "%s\n", description);
}

static constexpr const char * GL_ERROR_SEVERITY[] = {
    [GL_DEBUG_SEVERITY_HIGH] = "HIGH",
    [GL_DEBUG_SEVERITY_MEDIUM] = "MEDIUM",
    [GL_DEBUG_SEVERITY_LOW] = "LOW",
    [GL_DEBUG_SEVERITY_NOTIFICATION] = "INFO",
};

static constexpr const char * GL_ERROR_TYPE[] = {
    [GL_DEBUG_TYPE_ERROR] = "ERROR",
    [GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR] = "DEPRECATED_BEHAVIOR",
    [GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR] = "UNDEFINED_BEHAVIOR",
    [GL_DEBUG_TYPE_PORTABILITY] = "TYPE_PORTABILITY",
    [GL_DEBUG_TYPE_PERFORMANCE] = "PERFORMANCE",
    [GL_DEBUG_TYPE_PUSH_GROUP] = "PUSH_GROUP",
    [GL_DEBUG_TYPE_POP_GROUP] = "POP_GROUP",
    [GL_DEBUG_TYPE_OTHER] = "OTHER",
};

static void GLAPIENTRY
MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
    fprintf(stderr, "OpenGL: %s - %s - %s\n", GL_ERROR_TYPE[type], GL_ERROR_SEVERITY[severity], message);
    if (type == GL_DEBUG_TYPE_ERROR && severity == GL_DEBUG_SEVERITY_HIGH)
        abort();
}

Context::Context(int width, int height, const char *title, int maximized, int samples)
{
    if(glfwPlatformSupported(GLFW_PLATFORM_WIN32)) glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_WIN32);
    else if(glfwPlatformSupported(GLFW_PLATFORM_COCOA)) glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_COCOA);
    else if(glfwPlatformSupported(GLFW_PLATFORM_X11)) glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
    else if(glfwPlatformSupported(GLFW_PLATFORM_WAYLAND)) glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_WAYLAND);
    else {
        fprintf(stderr, "Error: could not find acceptable platform for GLFW\n");
        abort();
    }


    if (!glfwInit()) {
        std::cout << "Failed to initialize GLFW!" << std::endl;
        return;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_MAXIMIZED , maximized);
    glfwWindowHint(GLFW_SAMPLES, samples);

    glfwSetErrorCallback(GLFW_error);

    window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        abort();
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // enable vsync

    // callbacks
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetCursorEnterCallback(window, cursor_enter_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // user pointer to Context
    glfwSetWindowUserPointer(window, this);

    int version = gladLoadGL(glfwGetProcAddress);
    if (version == 0) {
        std::cout << "Failed to initialize OpenGL context" << std::endl;
        return;
    }

    // During init, enable debug output
    glEnable              ( GL_DEBUG_OUTPUT );
    glDebugMessageCallback( MessageCallback, 0 );

    imguiInit();

    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);
    _mouse_x = mouseX;
    _mouse_y = mouseY;
}

Context::~Context()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
};

void Context::run()
{
    double start_time = glfwGetTime();
    double last_frame_time = start_time;

    while (!glfwWindowShouldClose(window))
    {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        double time = glfwGetTime();
        double time_since_start = time - start_time;
        double delta_time = time - last_frame_time;
        last_frame_time = time;

        _current_view->onUpdate(time_since_start, delta_time);
        _current_view->onDraw(time_since_start, delta_time);

        swapBuffers();
        glfwPollEvents();
    }

    _current_view->onHideView();
}

void Context::swapBuffers()
{
    glfwSwapBuffers(window);
}

void Context::imguiNewFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void Context::imguiRender()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Context::imguiInit()
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGuiContext* imgui_context = ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();

    // io.FontGlobalScale = 1.85f; // Scale everything
}

void Context::showView(View& view)
{
    _current_view->onHideView();
    _current_view = &view;
    view.onShowView();

    // call resize callback on first frame
    glfwGetWindowSize(window, &width, &height);
    framebuffer_size_callback(window, width, height);
}

void Context::setVsync(bool enable)
{
    glfwSwapInterval(enable ? 1 : 0);
}

void Context::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    Context* ctx = (Context*)glfwGetWindowUserPointer(window);

    if (key >= 0 && key < GLFW_KEY_LAST) {
        ctx->keystate[key] = action > 0 ? 1 : 0;
    }

    // call pressed/release events (need to do that because of key repeat)
    if (action == GLFW_PRESS)
        ctx->_current_view->onKeyPress(key);
    else if (action == GLFW_RELEASE)
        ctx->_current_view->onKeyRelease(key);
}

void Context::cursor_position_callback(GLFWwindow* window, double x, double y)
{
    Context* ctx = (Context*)glfwGetWindowUserPointer(window);

    float dx = x - ctx->_mouse_x;
    float dy = y - ctx->_mouse_y;

    ctx->_current_view->onMouseMotion(x, y, dx, dy);

    ctx->_mouse_x = x;
    ctx->_mouse_y = y;

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        ctx->_current_view->onMouseDrag(x, y, dx, dy);
    }
}

void Context::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    Context* ctx = (Context*)glfwGetWindowUserPointer(window);

    if (action == GLFW_PRESS)
        ctx->_current_view->onMousePress(ctx->_mouse_x, ctx->_mouse_y, button);
    else if (action == GLFW_RELEASE)
        ctx->_current_view->onMouseRelease(ctx->_mouse_x, ctx->_mouse_y, button);
}

void Context::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    Context* ctx = (Context*)glfwGetWindowUserPointer(window);

    ctx->_current_view->onMouseScroll(xoffset, yoffset);
}

void Context::cursor_enter_callback(GLFWwindow* window, int entered)
{
    Context* ctx = (Context*)glfwGetWindowUserPointer(window);

    if (entered)
        ctx->_current_view->onMouseEnter(ctx->_mouse_x, ctx->_mouse_y);
    else
        ctx->_current_view->onMouseLeave(ctx->_mouse_x, ctx->_mouse_y);
}

void Context::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    Context* ctx = (Context*)glfwGetWindowUserPointer(window);

    ctx->width = width;
    ctx->height = height;
    ctx->_current_view->onResize(width, height);
}
