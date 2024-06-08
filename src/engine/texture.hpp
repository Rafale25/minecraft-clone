#pragma once

#include <glad/gl.h>

class Texture {
    public:
        Texture(GLsizei width, GLsizei height, GLenum format, GLenum min_filter, GLenum mag_filter, GLenum wrap);
        Texture(GLsizei width, GLsizei height, GLenum format, GLenum min_filter, GLenum mag_filter, GLenum wrap, const float borderColor[4]);

    public:
        GLsizei _width, _height;
        GLuint _texture;
};
