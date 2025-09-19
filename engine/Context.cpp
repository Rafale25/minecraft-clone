#include <cstdio>
#include <unordered_map>

#include <GLFW/glfw3.h>
#include <glad/gl.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "Context.hpp"
#include "Logger.hpp"
#include "profiler.hpp"

static void GLFW_error(int error, const char* description)
{
    fprintf(stderr, "%s\n", description);
}

static std::string errorSeverity(int severity) {
    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:            return "HIGH";
        case GL_DEBUG_SEVERITY_MEDIUM:          return "MEDIUM";
        case GL_DEBUG_SEVERITY_LOW:             return "LOW";
        case GL_DEBUG_SEVERITY_NOTIFICATION:    return "INFO";
        default:                                return "UNKNOWN";
    }
}

static std::string errorType(int type) {
    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:               return "ERROR";
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "DEPRECATED_BEHAVIOR";
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  return "UNDEFINED_BEHAVIOR";
        case GL_DEBUG_TYPE_PORTABILITY:         return "TYPE_PORTABILITY";
        case GL_DEBUG_TYPE_PERFORMANCE:         return "PERFORMANCE";
        case GL_DEBUG_TYPE_PUSH_GROUP:          return "PUSH_GROUP";
        case GL_DEBUG_TYPE_POP_GROUP:           return "POP_GROUP";
        case GL_DEBUG_TYPE_OTHER:               return "OTHER";
        default:                                return "UNKNOWN";
    }
}

static void GLAPIENTRY
MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
    if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) return;

    fprintf(stderr, "[OpenGL %s] - %s - %s\n", errorType(type).c_str(), errorSeverity(severity).c_str(), message);
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
        exit(-1);
    }

    if (!glfwInit()) {
        logF("Failed to initialize GLFW");
        exit(-1);
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
        logF("Failed to create GLFW window");
        glfwTerminate();
        exit(-1);
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
        logF("Failed to initialized OpenGL context");
        exit(-1);
    }

    // During init, enable debug output
    // glEnable(GL_DEBUG_OUTPUT); // Cause OpenGL Error on AMD ???
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback( MessageCallback, 0 );
    // glDebugMessageControl // https://www.khronos.org/opengl/wiki/Debug_Output#Getting_messages

    imguiInit();

    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);
    _mouse_x = mouseX;
    _mouse_y = mouseY;

    this->width = width;
    this->height = height;
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
        double delta_time = time - last_frame_time; // TODO: limit max delta time if it gets too laggy
        last_frame_time = time;

        legit::Profiler::beginFrame();

        _current_view->onUpdate(time_since_start, delta_time);

        imguiNewFrame();
        _current_view->onDraw(time_since_start, delta_time);

        legit::Profiler::endFrame();
        imguiRender();


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

    { // Automatically load and store font at window scale
        static std::unordered_map<float, ImFont*> fonts;

        float window_xscale, window_yscale;
        glfwGetWindowContentScale(window, &window_xscale, &window_yscale);
        const float dpi = window_xscale - 0.5f;
        ImGui::GetStyle().FontScaleDpi = dpi;

        if (!fonts.contains(dpi)) {
            const ImGuiIO& io = ImGui::GetIO();
            constexpr float baseFontSize = 13.0f;  // ImGui's default size
            ImFont* font = io.Fonts->AddFontFromFileTTF(
                // "./submodules/imgui/misc/fonts/ProggyClean.ttf",
                RESSOURCE_PATH "ProggyClean.ttf",
                baseFontSize * dpi
            );
            fonts[dpi] = font;

            logD("loaded font at scale {}", dpi);
        }

        ImGui::PushFont(fonts.at(dpi));
    }
}

void Context::imguiRender()
{
    ImGui::PopFont();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Context::imguiInit()
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
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
