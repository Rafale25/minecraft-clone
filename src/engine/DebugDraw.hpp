#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>
#include "Program.h"

class DebugDraw {
public:
    void drawLine(const glm::vec3& a, const glm::vec3& b, const glm::vec3& color = {1.0f, 0.0f, 0.0f});
    void drawRay(const glm::vec3& start, const glm::vec3& v, const glm::vec3& color = {1.0f, 0.0f, 0.0f});

    void drawCube(const glm::vec3 &center, float size, const glm::vec3 &color = {1.0f, 0.0f, 0.0f});
    void drawCuboid(const glm::vec3& center, const glm::vec3& extents, const glm::vec3& color = {1.0f, 0.0f, 0.0f});

    // void drawSphere();

    void drawAndFlush(const glm::mat4& view_projection);

    static DebugDraw& instance() {
        static DebugDraw instance;
        return instance;
    }

private:
    DebugDraw();
    ~DebugDraw() = default;

    DebugDraw(const DebugDraw&) = delete;
    DebugDraw& operator=(const DebugDraw&) = delete;
    DebugDraw(DebugDraw&&) = delete;
    DebugDraw& operator=(DebugDraw&&) = delete;

private:
    Program _program{"./assets/shaders/debug_draw/line.vert", "./assets/shaders/debug_draw/line.frag"};
    GLuint _vao, _vbo;
    GLuint _buffer;
    std::vector<glm::vec3> _vertices;
};
