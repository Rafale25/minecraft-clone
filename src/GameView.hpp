#pragma once

#include "View.hpp"
#include "Camera.hpp"
#include "Program.h"

// typedef struct Chunk
// {
    // int x, y;
    // GLuint VAO;
    // Blocktype blocks[4096];
// } Chunk_t;

class GameView: public View {
    public:
        GameView(Context& ctx): View(ctx)
        {
            int width, height;
            glfwGetWindowSize(ctx.window, &width, &height);

            camera = new OrbitCamera(
                glm::vec3(0.0f), 0.0f, 0.0f, 10.0f,
                60.0f, (float)width / (float)height, 0.01f, 1000.0f
            );

            cube_shader = new Program("./assets/shaders/cube.vs", "./assets/shaders/cube.fs");

            // float vertices[] = {
            //     0.0f, 0.0f, 0.0f,
            //     0.0f, 0.0f, 1.0f,
            //     0.0f, 1.0f, 0.0f
            // };

            // int indices[] = {
            //     0, 1, 2
            // };

            GLuint VBO, EBO;
            glGenVertexArrays(1, &VAO);
            // glGenBuffers(1, &VBO);
            // glGenBuffers(1, &EBO);

            glBindVertexArray(VAO);

            // glBindBuffer(GL_ARRAY_BUFFER, VBO);
            // glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

            // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

            // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
            // glEnableVertexAttribArray(0);

            // glBindVertexArray(0);

        }

        void onUpdate(float time_since_start, float dt)
        {
        }

        void onDraw(float time_since_start, float dt)
        {
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);

            glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            cube_shader->use();
            cube_shader->setMat4("u_projectionMatrix", camera->getProjection());
            cube_shader->setMat4("u_viewMatrix", camera->getView());

            glDrawArrays(GL_TRIANGLES, 0, 3*100);

            // glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
            // glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);

            gui(dt);
        }

        void gui(float dt)
        {
            ctx.imguiNewFrame();
            ImGui::Begin("Debug");

            ImGui::Text("%.4f ms", dt);
            ImGui::Text("%.2f fps", 1.0f / dt);

            glm::vec3 camera_pos = camera->getPosition();
            ImGui::Text("center: %.2f, %.2f, %.2f", camera_pos.x, camera_pos.y, camera_pos.z);
            ImGui::Text("yaw: %.2f", camera->getYaw());
            ImGui::Text("pitch: %.2f", camera->getPitch());

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
            if (ImGui::GetIO().WantCaptureMouse) return;

            camera->setYaw( camera->getYaw() - (dx * 0.005f) );
            camera->setPitch( camera->getPitch() + (dy * 0.005f) );
        }

        void onMousePress(int x, int y, int button)
        {
        }

        void onMouseRelease(int x, int y, int button)
        {
        }

        void onMouseScroll(int scroll_x, int scroll_y)
        {
            camera->setDistance( camera->getDistance() - (scroll_y * 0.2f) );
        }

        void onResize(int width, int height)
        {
            glViewport(0, 0, width, height);
        }

    private:
        OrbitCamera* camera;
        Program* cube_shader;

        GLuint VAO;
};
