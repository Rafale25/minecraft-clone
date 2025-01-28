#pragma once

#include <glad/gl.h>

struct Mesh {
    GLuint VAO, VBO, EBO;
    size_t count;
    int geometry_type = GL_TRIANGLES;
    bool has_element_buffer = false;
    bool released = false;

    void draw();
};

class Geometry {
    public:
        // TODO: add position and size args (https://moderngl-window.readthedocs.io/en/latest/reference/geometry.html)
        static Mesh quad_2d();

        // void quad_fs() {}
        // void sphere() {}
        // void bbox() {}
};
