#pragma once

#include "glm/ext/vector_float3.hpp"
#include <glad/gl.h>
#include <cstdint>

struct Mesh {
    GLuint VAO, VBO, EBO;
    uint64_t count;
    int32_t geometryType = GL_TRIANGLES;
    bool hasElementBuffer = false;
    bool released = false;

    void draw();
};

class Geometry {
    public:
        // TODO: add position and size args (https://moderngl-window.readthedocs.io/en/latest/reference/geometry.html)
        static Mesh quad_2d();
        static Mesh cube(const glm::vec3& size, const glm::vec3& center, bool normal=false, bool uvs=false);

        // void quad_fs() {}
        // void sphere() {}
        // void bbox() {}
};
