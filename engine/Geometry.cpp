#include "Geometry.hpp"
#include "VAO.hpp"
#include "glm/ext/vector_float2.hpp"

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
    const glm::vec3 s = size / 2.0f;

    if (!normals && !uvs) {
        const float vertices[] = {
            // -X
            center.x - s.x, center.y - s.y, center.z + s.z,
            center.x - s.x, center.y + s.y, center.z + s.z,
            center.x - s.x, center.y + s.y, center.z - s.z,
            center.x - s.x, center.y - s.y, center.z + s.z,
            center.x - s.x, center.y + s.y, center.z - s.z,
            center.x - s.x, center.y - s.y, center.z - s.z,

            // +X
            center.x + s.x, center.y - s.y, center.z - s.z,
            center.x + s.x, center.y + s.y, center.z - s.z,
            center.x + s.x, center.y - s.y, center.z + s.z,
            center.x + s.x, center.y + s.y, center.z - s.z,
            center.x + s.x, center.y + s.y, center.z + s.z,
            center.x + s.x, center.y - s.y, center.z + s.z,

            // -Y
            center.x + s.x, center.y - s.y, center.z - s.z,
            center.x + s.x, center.y - s.y, center.z + s.z,
            center.x - s.x, center.y - s.y, center.z + s.z,
            center.x + s.x, center.y - s.y, center.z - s.z,
            center.x - s.x, center.y - s.y, center.z + s.z,
            center.x - s.x, center.y - s.y, center.z - s.z,

            // +Y
            center.x + s.x, center.y + s.y, center.z - s.z,
            center.x - s.x, center.y + s.y, center.z - s.z,
            center.x + s.x, center.y + s.y, center.z + s.z,
            center.x - s.x, center.y + s.y, center.z - s.z,
            center.x - s.x, center.y + s.y, center.z + s.z,
            center.x + s.x, center.y + s.y, center.z + s.z,

            // -Z
            center.x + s.x, center.y + s.y, center.z - s.z,
            center.x + s.x, center.y - s.y, center.z - s.z,
            center.x - s.x, center.y - s.y, center.z - s.z,
            center.x + s.x, center.y + s.y, center.z - s.z,
            center.x - s.x, center.y - s.y, center.z - s.z,
            center.x - s.x, center.y + s.y, center.z - s.z,

            // +Z
            center.x + s.x, center.y - s.y, center.z + s.z,
            center.x + s.x, center.y + s.y, center.z + s.z,
            center.x - s.x, center.y - s.y, center.z + s.z,
            center.x + s.x, center.y + s.y, center.z + s.z,
            center.x - s.x, center.y + s.y, center.z + s.z,
            center.x - s.x, center.y - s.y, center.z + s.z,
        };

        Mesh mesh;
        mesh.hasElementBuffer = false;
        mesh.count = sizeof(vertices) / sizeof(float);
        mesh.geometryType = GL_TRIANGLES;
        mesh.VBO = createBufferStorage(&vertices[0], sizeof(vertices));
        mesh.VAO = createVAO(mesh.VBO, "3f");
        return mesh;
    }

    if (normals && !uvs) {
        const float vertices[] = {
                // -X
            center.x - s.x, center.y - s.y, center.z + s.z,    -1.0f, 0.0f, 0.0f,
            center.x - s.x, center.y + s.y, center.z + s.z,    -1.0f, 0.0f, 0.0f,
            center.x - s.x, center.y + s.y, center.z - s.z,    -1.0f, 0.0f, 0.0f,
            center.x - s.x, center.y - s.y, center.z + s.z,    -1.0f, 0.0f, 0.0f,
            center.x - s.x, center.y + s.y, center.z - s.z,    -1.0f, 0.0f, 0.0f,
            center.x - s.x, center.y - s.y, center.z - s.z,    -1.0f, 0.0f, 0.0f,

            // +X
            center.x + s.x, center.y - s.y, center.z - s.z,    1.0f, 0.0f, 0.0f,
            center.x + s.x, center.y + s.y, center.z - s.z,    1.0f, 0.0f, 0.0f,
            center.x + s.x, center.y - s.y, center.z + s.z,    1.0f, 0.0f, 0.0f,
            center.x + s.x, center.y + s.y, center.z - s.z,    1.0f, 0.0f, 0.0f,
            center.x + s.x, center.y + s.y, center.z + s.z,    1.0f, 0.0f, 0.0f,
            center.x + s.x, center.y - s.y, center.z + s.z,    1.0f, 0.0f, 0.0f,

            // -Y
            center.x + s.x, center.y - s.y, center.z - s.z,    0.0f, -1.0f, 0.0f,
            center.x + s.x, center.y - s.y, center.z + s.z,    0.0f, -1.0f, 0.0f,
            center.x - s.x, center.y - s.y, center.z + s.z,    0.0f, -1.0f, 0.0f,
            center.x + s.x, center.y - s.y, center.z - s.z,    0.0f, -1.0f, 0.0f,
            center.x - s.x, center.y - s.y, center.z + s.z,    0.0f, -1.0f, 0.0f,
            center.x - s.x, center.y - s.y, center.z - s.z,    0.0f, -1.0f, 0.0f,

            // +Y
            center.x + s.x, center.y + s.y, center.z - s.z,    0.0f, 1.0f, 0.0f,
            center.x - s.x, center.y + s.y, center.z - s.z,    0.0f, 1.0f, 0.0f,
            center.x + s.x, center.y + s.y, center.z + s.z,    0.0f, 1.0f, 0.0f,
            center.x - s.x, center.y + s.y, center.z - s.z,    0.0f, 1.0f, 0.0f,
            center.x - s.x, center.y + s.y, center.z + s.z,    0.0f, 1.0f, 0.0f,
            center.x + s.x, center.y + s.y, center.z + s.z,    0.0f, 1.0f, 0.0f,

            // -Z
            center.x + s.x, center.y + s.y, center.z - s.z,    0.0f, 0.0f, -1.0f,
            center.x + s.x, center.y - s.y, center.z - s.z,    0.0f, 0.0f, -1.0f,
            center.x - s.x, center.y - s.y, center.z - s.z,    0.0f, 0.0f, -1.0f,
            center.x + s.x, center.y + s.y, center.z - s.z,    0.0f, 0.0f, -1.0f,
            center.x - s.x, center.y - s.y, center.z - s.z,    0.0f, 0.0f, -1.0f,
            center.x - s.x, center.y + s.y, center.z - s.z,    0.0f, 0.0f, -1.0f,

            // +Z
            center.x + s.x, center.y - s.y, center.z + s.z,    0.0f, 0.0f, 1.0f,
            center.x + s.x, center.y + s.y, center.z + s.z,    0.0f, 0.0f, 1.0f,
            center.x - s.x, center.y - s.y, center.z + s.z,    0.0f, 0.0f, 1.0f,
            center.x + s.x, center.y + s.y, center.z + s.z,    0.0f, 0.0f, 1.0f,
            center.x - s.x, center.y + s.y, center.z + s.z,    0.0f, 0.0f, 1.0f,
            center.x - s.x, center.y - s.y, center.z + s.z,    0.0f, 0.0f, 1.0f,
        };

        Mesh mesh;
        mesh.hasElementBuffer = false;
        mesh.count = sizeof(vertices) / sizeof(float);
        mesh.geometryType = GL_TRIANGLES;
        mesh.VBO = createBufferStorage(&vertices[0], sizeof(vertices));
        mesh.VAO = createVAO(mesh.VBO, "3f 3f");
        return mesh;
    }

    const float vertices[] = {
        // -X
        center.x - s.x, center.y - s.y, center.z + s.z,    -1.0f, 0.0f, 0.0f,   1.0f, 0.0f,
        center.x - s.x, center.y + s.y, center.z + s.z,    -1.0f, 0.0f, 0.0f,   1.0f, 1.0f,
        center.x - s.x, center.y + s.y, center.z - s.z,    -1.0f, 0.0f, 0.0f,   0.0f, 1.0f,
        center.x - s.x, center.y - s.y, center.z + s.z,    -1.0f, 0.0f, 0.0f,   1.0f, 0.0f,
        center.x - s.x, center.y + s.y, center.z - s.z,    -1.0f, 0.0f, 0.0f,   0.0f, 1.0f,
        center.x - s.x, center.y - s.y, center.z - s.z,    -1.0f, 0.0f, 0.0f,   0.0f, 0.0f,

        // +X
        center.x + s.x, center.y - s.y, center.z - s.z,    1.0f, 0.0f, 0.0f,    1.0f, 0.0f,
        center.x + s.x, center.y + s.y, center.z - s.z,    1.0f, 0.0f, 0.0f,    1.0f, 1.0f,
        center.x + s.x, center.y - s.y, center.z + s.z,    1.0f, 0.0f, 0.0f,    0.0f, 0.0f,
        center.x + s.x, center.y + s.y, center.z - s.z,    1.0f, 0.0f, 0.0f,    1.0f, 1.0f,
        center.x + s.x, center.y + s.y, center.z + s.z,    1.0f, 0.0f, 0.0f,    0.0f, 1.0f,
        center.x + s.x, center.y - s.y, center.z + s.z,    1.0f, 0.0f, 0.0f,    0.0f, 0.0f,

        // -Y
        center.x + s.x, center.y - s.y, center.z - s.z,    0.0f, -1.0f, 0.0f,   1.0f, 0.0f,
        center.x + s.x, center.y - s.y, center.z + s.z,    0.0f, -1.0f, 0.0f,   1.0f, 1.0f,
        center.x - s.x, center.y - s.y, center.z + s.z,    0.0f, -1.0f, 0.0f,   0.0f, 1.0f,
        center.x + s.x, center.y - s.y, center.z - s.z,    0.0f, -1.0f, 0.0f,   1.0f, 0.0f,
        center.x - s.x, center.y - s.y, center.z + s.z,    0.0f, -1.0f, 0.0f,   0.0f, 1.0f,
        center.x - s.x, center.y - s.y, center.z - s.z,    0.0f, -1.0f, 0.0f,   0.0f, 0.0f,

        // +Y
        center.x + s.x, center.y + s.y, center.z - s.z,    0.0f, 1.0f, 0.0f,    1.0f, 0.0f,
        center.x - s.x, center.y + s.y, center.z - s.z,    0.0f, 1.0f, 0.0f,    0.0f, 0.0f,
        center.x + s.x, center.y + s.y, center.z + s.z,    0.0f, 1.0f, 0.0f,    1.0f, 1.0f,
        center.x - s.x, center.y + s.y, center.z - s.z,    0.0f, 1.0f, 0.0f,    0.0f, 0.0f,
        center.x - s.x, center.y + s.y, center.z + s.z,    0.0f, 1.0f, 0.0f,    0.0f, 1.0f,
        center.x + s.x, center.y + s.y, center.z + s.z,    0.0f, 1.0f, 0.0f,    1.0f, 1.0f,

        // -Z
        center.x + s.x, center.y + s.y, center.z - s.z,    0.0f, 0.0f, -1.0f,   0.0f, 1.0f,
        center.x + s.x, center.y - s.y, center.z - s.z,    0.0f, 0.0f, -1.0f,   0.0f, 0.0f,
        center.x - s.x, center.y - s.y, center.z - s.z,    0.0f, 0.0f, -1.0f,   1.0f, 0.0f,
        center.x + s.x, center.y + s.y, center.z - s.z,    0.0f, 0.0f, -1.0f,   0.0f, 1.0f,
        center.x - s.x, center.y - s.y, center.z - s.z,    0.0f, 0.0f, -1.0f,   1.0f, 0.0f,
        center.x - s.x, center.y + s.y, center.z - s.z,    0.0f, 0.0f, -1.0f,   1.0f, 1.0f,

        // +Z
        center.x + s.x, center.y - s.y, center.z + s.z,    0.0f, 0.0f, 1.0f,    1.0f, 0.0f,
        center.x + s.x, center.y + s.y, center.z + s.z,    0.0f, 0.0f, 1.0f,    1.0f, 1.0f,
        center.x - s.x, center.y - s.y, center.z + s.z,    0.0f, 0.0f, 1.0f,    0.0f, 0.0f,
        center.x + s.x, center.y + s.y, center.z + s.z,    0.0f, 0.0f, 1.0f,    1.0f, 1.0f,
        center.x - s.x, center.y + s.y, center.z + s.z,    0.0f, 0.0f, 1.0f,    0.0f, 1.0f,
        center.x - s.x, center.y - s.y, center.z + s.z,    0.0f, 0.0f, 1.0f,    0.0f, 0.0f,
    };

    Mesh mesh;
    mesh.hasElementBuffer = false;
    mesh.count = sizeof(vertices) / sizeof(float);
    mesh.geometryType = GL_TRIANGLES;
    mesh.VBO = createBufferStorage(&vertices[0], sizeof(vertices));
    mesh.VAO = createVAO(mesh.VBO, "3f 3f 2f");
    return mesh;
}
