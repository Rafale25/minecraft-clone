#include "DebugDraw.hpp"
#include "VAO.hpp"

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

void DebugDraw::drawAndFlush(const glm::mat4& view_projection)
{
    _program.use();
    _program.setMat4("u_viewProjection", view_projection);

    glNamedBufferSubData(_vbo, 0, sizeof(float) * 6 * _vertices.size(), (const void *)_vertices.data());
    glBindVertexArray(_vao);
    glDrawArrays(GL_LINES, 0, _vertices.size());
    _vertices.clear();
}

void DebugDraw::drawCube(const glm::vec3 &center, float size, const glm::vec3 &color)
{
    drawCube(center, glm::vec3(size*0.5f), color);
}

void DebugDraw::drawCube(const glm::vec3 &center, const glm::vec3 &extents, const glm::vec3 &color)
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
