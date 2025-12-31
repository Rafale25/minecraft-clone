#include "DebugDraw.hpp"
#include "VAO.hpp"
#include "Frustum.hpp"
#include "Logger.hpp"
#include <glad/gl.h>
#include <glm/gtc/constants.hpp>

static inline int32_t packColor(uint8_t r, uint8_t g, uint8_t b) {
    return (r << 16 | g << 8 | b);
}

static inline float intBitsToFloat(int32_t int_value) {
    union {
        int32_t i;
        float f;
    } bits;

    bits.i = int_value;
    return bits.f;
}

DebugDraw::DebugDraw()
{
    constexpr int initial_size = 10'000 * sizeof(float) * 4;
    _vbo = createBufferData(nullptr, initial_size, GL_DYNAMIC_DRAW);
    _vao = createVAO(_vbo, "3f 1i");
}

void DebugDraw::drawLine(const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &color)
{
    _vertices.emplace_back(a.x);
    _vertices.emplace_back(a.y);
    _vertices.emplace_back(a.z);
    _vertices.emplace_back(intBitsToFloat(packColor(color.r * 255, color.g * 255, color.b * 255)));

    _vertices.emplace_back(b.x);
    _vertices.emplace_back(b.y);
    _vertices.emplace_back(b.z);
    _vertices.emplace_back(intBitsToFloat(packColor(color.r * 255, color.g * 255, color.b * 255)));
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

void DebugDraw::drawCuboidMinMax(const glm::vec3 &min, const glm::vec3 &max, const glm::vec3 &color)
{
    drawCuboid((max + min) / 2.0f, (max - min) * 0.5f, color);
}

void DebugDraw::drawSphere(const glm::vec3 &center, float radius, const glm::vec3& color)
{
    constexpr int32_t resolution = 32;

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
    const std::vector<glm::vec3> points = extractFrustumCornersWorldSpace(view_projection);

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

void DebugDraw::draw(const glm::mat4& view_projection) {
    const int32_t vertex_count = _vertices.size() / 4; // x y z packedColor
    const int32_t vertices_size_bytes = _vertices.size() * sizeof(float);

    int32_t buffer_size = -1;
    glGetNamedBufferParameteriv(_vbo, GL_BUFFER_SIZE, &buffer_size);

    if (buffer_size < vertices_size_bytes) {
        glNamedBufferData(_vbo, vertices_size_bytes, (const void *)_vertices.data(), GL_DYNAMIC_DRAW);
    } else {
        glNamedBufferSubData(_vbo, 0, vertices_size_bytes, (const void *)_vertices.data());
    }

    _program.use();
    _program.setMat4("u_viewProjection", view_projection);

    float line_width; // save state
    glGetFloatv(GL_LINE_WIDTH, &line_width);
    glLineWidth(2.0f);

    glBindVertexArray(_vao);
    glDrawArrays(GL_LINES, 0, vertex_count);

    glLineWidth(line_width);
}

void DebugDraw::flush() {
    _vertices.clear();
}

void DebugDraw::drawAndFlush(const glm::mat4& view_projection)
{
    draw(view_projection);
    flush();
}
