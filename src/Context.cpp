#include "Context.hpp"

Context::Context(int width, int height, const char *title)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
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

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
    }

    imguiInit();

    // default View
    _current_view = new DefaultView(*this);

    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);
    _mouseX = mouseX;
    _mouseY = mouseY;
}

Context::~Context()
{
    glfwTerminate();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
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

    io.FontGlobalScale = 1.85f; // Scale everything
}

void Context::show_view(View *view)
{
    _current_view->onHideView();
    _current_view = view;
    view->onShowView();
}

void Context::setVsync(int value)
{
    // TODO: add check for invalid value
    glfwSwapInterval(value);
}

void Context::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    Context* ctx = ((Context*)glfwGetWindowUserPointer(window));

    // if (key >= 0 && key < GLFW_KEY_LAST) {
        // ctx->keyState[key] = action;
    // }

    // call pressed/release events (need to do that because of key repeat)
    if (action == GLFW_PRESS)
        ctx->_current_view->onKeyPress(key);
    else if (action == GLFW_RELEASE)
        ctx->_current_view->onKeyRelease(key);
}

void Context::cursor_position_callback(GLFWwindow* window, double x, double y)
{
    Context* ctx = ((Context*)glfwGetWindowUserPointer(window));

    float dx = x - ctx->_mouseX;
    float dy = y - ctx->_mouseY;

    ctx->_current_view->onMouseMotion(x, y, dx, dy);

    ctx->_mouseX = x;
    ctx->_mouseY = y;

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        ctx->_current_view->onMouseDrag(x, y, dx, dy);
    }
}

void Context::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    Context* ctx = ((Context*)glfwGetWindowUserPointer(window));

    if (action == GLFW_PRESS)
        ctx->_current_view->onMousePress(ctx->_mouseX, ctx->_mouseY, button);
    else if (action == GLFW_RELEASE)
        ctx->_current_view->onMouseRelease(ctx->_mouseX, ctx->_mouseY, button);
}

void Context::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    Context* ctx = ((Context*)glfwGetWindowUserPointer(window));

    ctx->_current_view->onMouseScroll(xoffset, yoffset);
}

void Context::cursor_enter_callback(GLFWwindow* window, int entered)
{
    Context* ctx = ((Context*)glfwGetWindowUserPointer(window));

    if (entered)
        ctx->_current_view->onMouseEnter(ctx->_mouseX, ctx->_mouseY);
    else
        ctx->_current_view->onMouseLeave(ctx->_mouseX, ctx->_mouseY);
}

void Context::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    Context* ctx = ((Context*)glfwGetWindowUserPointer(window));

    ctx->_current_view->onResize(width, height);
}
