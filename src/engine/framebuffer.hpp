#pragma once

#include <glad/gl.h>

class Framebuffer {
    public:
        Framebuffer(GLenum draw_buffer, GLenum read_buffer);

        void bind();
        void attachTexture(GLuint texture, GLenum attachment);

    private:
        GLuint _framebuffer;
};
