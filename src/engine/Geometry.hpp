#pragma once

#include <glad/gl.h>
#include "VAO.hpp"

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
                -1.0f,  1.0f,     0.0f, 1.0f,
                 1.0f,  1.0f,     1.0f, 1.0f,
                -1.0f, -1.0f,     0.0f, 0.0f,
                 1.0f, -1.0f,     1.0f, 0.0f,
            };

            mesh.VBO = createBufferStorage(&vertices[0], sizeof(vertices));
            mesh.VAO = createVAO(mesh.VBO, "2f 2f");

            return mesh;
        }

        void quad_fs() {}
        void sphere() {}
        void bbox() {}
};
