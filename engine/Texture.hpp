#pragma once

#include <glad/gl.h>

class Texture {
    struct RGBA {
        GLint r, g, b, a;
    };

    public:
        Texture() = default;
        Texture(GLsizei width, GLsizei height, GLenum format, GLenum min_filter, GLenum mag_filter, GLenum wrap);
        Texture(GLsizei width, GLsizei height, GLenum format, GLenum min_filter, GLenum mag_filter, GLenum wrap, const float borderColor[4]);
        void setSwizzle(RGBA rgba);
        void destroy();

    public:
        GLsizei _width = 0, _height = 0;
        GLuint _texture = 0;
};
