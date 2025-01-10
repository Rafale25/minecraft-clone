#pragma once

#include <glad/gl.h>

class Framebuffer {
    public:
        Framebuffer();
        Framebuffer(GLenum draw_buffer, GLenum read_buffer);

        void bind();
        void attachTexture(GLuint texture, GLenum attachment);
        void destroy();

    private:
        GLuint _framebuffer;
};
