#include "Geometry.hpp"
#include "VAO.hpp"

void Mesh::draw() {
    glBindVertexArray(VAO);

    if (hasElementBuffer)
        glDrawElements(geometryType, count, GL_UNSIGNED_INT, 0);
    else
        glDrawArrays(geometryType, 0, count);
}

Mesh Geometry::quad_2d() {
    Mesh mesh;
    mesh.hasElementBuffer = false;
    mesh.count = 4;
    mesh.geometryType = GL_TRIANGLE_STRIP;

    const float vertices[] = {
        // positions      // texture Coords
         1.0f, -1.0f,     1.0f, 0.0f,
         1.0f,  1.0f,     1.0f, 1.0f,
        -1.0f, -1.0f,     0.0f, 0.0f,
        -1.0f,  1.0f,     0.0f, 1.0f,
    };

    mesh.VBO = createBufferStorage(&vertices[0], sizeof(vertices));
    mesh.VAO = createVAO(mesh.VBO, "2f 2f");

    return mesh;
}

Mesh Geometry::cube(const glm::vec3& size, const glm::vec3& center, bool normals, bool uvs) {
    const auto s = size / 2.0f;

    const float positions[] = {
        center.x + s.x, center.y - s.y, center.z + s.z,
        center.x + s.x, center.y + s.y, center.z + s.z,
        center.x - s.x, center.y - s.y, center.z + s.z,
        center.x + s.x, center.y + s.y, center.z + s.z,
        center.x - s.x, center.y + s.y, center.z + s.z,
        center.x - s.x, center.y - s.y, center.z + s.z,
        center.x + s.x, center.y - s.y, center.z - s.z,
        center.x + s.x, center.y + s.y, center.z - s.z,
        center.x + s.x, center.y - s.y, center.z + s.z,
        center.x + s.x, center.y + s.y, center.z - s.z,
        center.x + s.x, center.y + s.y, center.z + s.z,
        center.x + s.x, center.y - s.y, center.z + s.z,
        center.x + s.x, center.y - s.y, center.z - s.z,
        center.x + s.x, center.y - s.y, center.z + s.z,
        center.x - s.x, center.y - s.y, center.z + s.z,
        center.x + s.x, center.y - s.y, center.z - s.z,
        center.x - s.x, center.y - s.y, center.z + s.z,
        center.x - s.x, center.y - s.y, center.z - s.z,
        center.x - s.x, center.y - s.y, center.z + s.z,
        center.x - s.x, center.y + s.y, center.z + s.z,
        center.x - s.x, center.y + s.y, center.z - s.z,
        center.x - s.x, center.y - s.y, center.z + s.z,
        center.x - s.x, center.y + s.y, center.z - s.z,
        center.x - s.x, center.y - s.y, center.z - s.z,
        center.x + s.x, center.y + s.y, center.z - s.z,
        center.x + s.x, center.y - s.y, center.z - s.z,
        center.x - s.x, center.y - s.y, center.z - s.z,
        center.x + s.x, center.y + s.y, center.z - s.z,
        center.x - s.x, center.y - s.y, center.z - s.z,
        center.x - s.x, center.y + s.y, center.z - s.z,
        center.x + s.x, center.y + s.y, center.z - s.z,
        center.x - s.x, center.y + s.y, center.z - s.z,
        center.x + s.x, center.y + s.y, center.z + s.z,
        center.x - s.x, center.y + s.y, center.z - s.z,
        center.x - s.x, center.y + s.y, center.z + s.z,
        center.x + s.x, center.y + s.y, center.z + s.z,
    };

    Mesh mesh;
    mesh.hasElementBuffer = false;
    mesh.count = sizeof(positions) / sizeof(float);
    mesh.geometryType = GL_TRIANGLES;
    mesh.VBO = createBufferStorage(&positions[0], sizeof(positions));
    mesh.VAO = createVAO(mesh.VBO, "3f");

    return mesh;
}
