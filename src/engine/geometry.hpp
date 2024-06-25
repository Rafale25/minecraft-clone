#pragma once

#include <glad/gl.h>

struct Mesh {
    GLuint VAO, VBO, EBO;
    size_t count;
    int geometry_type = GL_TRIANGLES;
    bool has_element_buffer = false;
    bool released = false;

    void draw() {
        glBindVertexArray(VAO);

        if (has_element_buffer)
            glDrawElements(geometry_type, count, GL_UNSIGNED_INT, 0);
        else
            glDrawArrays(geometry_type, 0, count);
    }
};

class Geometry {
    public:
        static Mesh quad_2d() { // TODO: add position and size args (https://moderngl-window.readthedocs.io/en/latest/reference/geometry.html)
            Mesh mesh;
            mesh.has_element_buffer = false;
            mesh.count = 4;
            mesh.geometry_type = GL_TRIANGLE_STRIP;

            const float vertices[] = {
              // positions        // texture Coords
                -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
                -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
                 1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
                 1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
            };
            glCreateVertexArrays(1, &mesh.VAO);
            glCreateBuffers(1, &mesh.VBO);

            glEnableVertexArrayAttrib(mesh.VAO, 0);
            glVertexArrayAttribBinding(mesh.VAO, 0, 0);
            glVertexArrayAttribFormat(mesh.VAO, 0, 3, GL_FLOAT, GL_FALSE, 0);

            glEnableVertexArrayAttrib(mesh.VAO, 1);
            glVertexArrayAttribBinding(mesh.VAO, 1, 0);
            glVertexArrayAttribFormat(mesh.VAO, 1, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT));

            glNamedBufferData(mesh.VBO, sizeof(vertices), &vertices[0], GL_STATIC_DRAW);
            glVertexArrayVertexBuffer(mesh.VAO, 0, mesh.VBO, 0, 5 * sizeof(GL_FLOAT));

            return mesh;
        }

        void quad_fs() {}
        void sphere() {}
        void bbox() {}
};
