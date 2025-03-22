#include <glad/gl.h>
#include "Framebuffer.hpp"

Framebuffer::Framebuffer() {
    glCreateFramebuffers(1, &_framebuffer);
}

Framebuffer::Framebuffer(GLenum draw_buffer, GLenum read_buffer) {
    glCreateFramebuffers(1, &_framebuffer);

    glNamedFramebufferDrawBuffer(_framebuffer, draw_buffer);
    glNamedFramebufferReadBuffer(_framebuffer, read_buffer);
}

void Framebuffer::bind() {
    glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);
}

void Framebuffer::attachTexture(GLuint texture, GLenum attachment) {
    glNamedFramebufferTexture(_framebuffer, attachment, texture, 0);
}

void Framebuffer::destroy() {
    glDeleteFramebuffers(1, &_framebuffer);
}
