#include "Geometry.hpp"
#include "VAO.hpp"

void Mesh::draw() {
    glBindVertexArray(VAO);

    if (has_element_buffer)
        glDrawElements(geometry_type, count, GL_UNSIGNED_INT, 0);
    else
        glDrawArrays(geometry_type, 0, count);
}

Mesh Geometry::quad_2d() {
    Mesh mesh;
    mesh.has_element_buffer = false;
    mesh.count = 4;
    mesh.geometry_type = GL_TRIANGLE_STRIP;

    const float vertices[] = {
        // positions        // texture Coords
        -1.0f,  1.0f,     0.0f, 1.0f,
            1.0f,  1.0f,     1.0f, 1.0f,
        -1.0f, -1.0f,     0.0f, 0.0f,
            1.0f, -1.0f,     1.0f, 0.0f,
    };

    mesh.VBO = createBufferStorage(&vertices[0], sizeof(vertices));
    mesh.VAO = createVAO(mesh.VBO, "2f 2f");

    return mesh;
}
