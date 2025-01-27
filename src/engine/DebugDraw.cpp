#include "DebugDraw.hpp"
#include "VAO.hpp"
#include "Frustum.hpp"
#include <glm/gtc/constants.hpp>

DebugDraw::DebugDraw()
{
    constexpr int MAX_SIZE = sizeof(float) * 10'000;

    _vbo = createBufferStorage(nullptr, MAX_SIZE, GL_DYNAMIC_STORAGE_BIT);
    _vao = createVAO(_vbo, "3f 3f");
}

void DebugDraw::drawLine(const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &color)
{
    // TODO: add check for max size

    _vertices.push_back(a);
    _vertices.push_back(color);
    _vertices.push_back(b);
    _vertices.push_back(color);
}

void DebugDraw::drawRay(const glm::vec3 &start, const glm::vec3 &v, const glm::vec3 &color)
{
    drawLine(start, start + v, color);
}

void DebugDraw::drawCube(const glm::vec3 &center, float size, const glm::vec3 &color)
{
    drawCuboid(center, glm::vec3(size*0.5f), color);
}

void DebugDraw::drawCuboid(const glm::vec3 &center, const glm::vec3 &extents, const glm::vec3 &color)
{
    // -Y
    const glm::vec3 x0y0z0 = center + glm::vec3{-extents.x, -extents.y, -extents.z};
    const glm::vec3 x1y0z0 = center + glm::vec3{extents.x, -extents.y, -extents.z};
    const glm::vec3 x0y0z1 = center + glm::vec3{-extents.x, -extents.y, extents.z};
    const glm::vec3 x1y0z1 = center + glm::vec3{extents.x, -extents.y, extents.z};

    // +Y
    const glm::vec3 x0y1z0 = center + glm::vec3{-extents.x, extents.y, -extents.z};
    const glm::vec3 x1y1z0 = center + glm::vec3{extents.x, extents.y, -extents.z};
    const glm::vec3 x0y1z1 = center + glm::vec3{-extents.x, extents.y, extents.z};
    const glm::vec3 x1y1z1 = center + glm::vec3{extents.x, extents.y, extents.z};

    drawLine(x0y0z0, x1y0z0, color);
    drawLine(x0y0z0, x0y0z1, color);
    drawLine(x1y0z1, x1y0z0, color);
    drawLine(x1y0z1, x0y0z1, color);

    drawLine(x0y1z0, x1y1z0, color);
    drawLine(x0y1z0, x0y1z1, color);
    drawLine(x1y1z1, x1y1z0, color);
    drawLine(x1y1z1, x0y1z1, color);

    drawLine(x0y0z0, x0y1z0, color);
    drawLine(x1y0z0, x1y1z0, color);
    drawLine(x0y0z1, x0y1z1, color);
    drawLine(x1y0z1, x1y1z1, color);
}

void DebugDraw::drawSphere(const glm::vec3 &center, float radius, const glm::vec3& color)
{
    constexpr int resolution = 32;

    for (int32_t i = 0 ; i < resolution ; ++i) {
        const float theta0 = (float)i / (float)resolution * glm::tau<float>();
        const float theta1 = (float)(i+1) / (float)resolution * glm::tau<float>();

        drawLine(
            center + glm::vec3(cos(theta0), 0.0f, sin(theta0)) * radius,
            center + glm::vec3(cos(theta1), 0.0f, sin(theta1)) * radius);

        drawLine(
            center + glm::vec3(cos(theta0), sin(theta0), 0.0f) * radius,
            center + glm::vec3(cos(theta1), sin(theta1), 0.0f) * radius);

        drawLine(
            center + glm::vec3(0.0f, sin(theta0), cos(theta0)) * radius,
            center + glm::vec3(0.0f, sin(theta1), cos(theta1)) * radius);
    }
}

void DebugDraw::drawFrustum(const glm::mat4 &view_projection, const glm::vec3& color)
{
    const std::vector<glm::vec4> points = extractFrustumCornersWorldSpace(view_projection);

    drawLine(points[0], points[1], color);
    drawLine(points[0], points[2], color);
    drawLine(points[3], points[1], color);
    drawLine(points[3], points[2], color);

    drawLine(points[0], points[4], color);
    drawLine(points[1], points[5], color);
    drawLine(points[2], points[6], color);
    drawLine(points[3], points[7], color);

    drawLine(points[4], points[5], color);
    drawLine(points[4], points[6], color);
    drawLine(points[7], points[5], color);
    drawLine(points[7], points[6], color);
}

void DebugDraw::drawAndFlush(const glm::mat4& view_projection)
{
    const size_t vertex_count = _vertices.size() / 2; // position, color

    _program.use();
    _program.setMat4("u_viewProjection", view_projection);

    glNamedBufferSubData(_vbo, 0, sizeof(float) * 6 * vertex_count, (const void *)_vertices.data());
    glBindVertexArray(_vao);
    glDrawArrays(GL_LINES, 0, vertex_count);
    _vertices.clear();
}
