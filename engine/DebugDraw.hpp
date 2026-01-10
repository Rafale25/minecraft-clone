#pragma once

#include "ShaderProgram.hpp"
#include <glm/detail/type_vec3.hpp>
#include <glm/detail/type_mat4x4.hpp>

constexpr glm::vec3 DEFAULT_COLOR = {1.0f, 0.0f, 0.0f};

class DebugDraw {
public:
    void drawLine(const glm::vec3& a, const glm::vec3& b, const glm::vec3& color = DEFAULT_COLOR);
    void drawRay(const glm::vec3& start, const glm::vec3& v, const glm::vec3& color = DEFAULT_COLOR);
    void drawCube(const glm::vec3 &center, float size = 1.0f, const glm::vec3 &color = DEFAULT_COLOR);
    void drawCuboid(const glm::vec3& center, const glm::vec3& extents = {0.5f, 0.5f, 0.5f}, const glm::vec3& color = DEFAULT_COLOR);
    void drawCuboidMinMax(const glm::vec3& min, const glm::vec3& max, const glm::vec3& color = DEFAULT_COLOR);
    void drawSphere(const glm::vec3& center, float radius = 1.0f, const glm::vec3& color = DEFAULT_COLOR);
    void drawFrustum(const glm::mat4& view_projection, const glm::vec3& color = DEFAULT_COLOR);

    void draw(const glm::mat4& view_projection);
    void flush();
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
    ShaderProgram m_program{ENGINE_RESSOURCE_PATH "debug_draw/line.vert", ENGINE_RESSOURCE_PATH "debug_draw/line.frag"};
    GLuint m_vao, m_vbo;
    std::vector<float> m_vertices;
};
