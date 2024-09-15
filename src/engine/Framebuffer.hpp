#pragma once

#include <glad/gl.h>

class Framebuffer {
    public:
        Framebuffer(GLenum draw_buffer=GL_FRONT, GLenum read_buffer=GL_FRONT);

        void bind();
        void attachTexture(GLuint texture, GLenum attachment);

    private:
        GLuint _framebuffer;
};
