#pragma once

typedef unsigned int GLenum;
typedef unsigned int GLuint;

class Framebuffer {
    public:
        Framebuffer();
        Framebuffer(GLenum draw_buffer, GLenum read_buffer);

        void bind();
        void drawBuffers(int n, const GLenum *bufs);
        void attachTexture(GLuint texture, GLenum attachment);
        void destroy();

    // private:
        GLuint m_framebuffer = 0;
};
