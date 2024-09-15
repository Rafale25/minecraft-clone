#include "Framebuffer.hpp"

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
