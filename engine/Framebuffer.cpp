#include "Framebuffer.hpp"
#include <glad/gl.h>

Framebuffer::Framebuffer() {
    glCreateFramebuffers(1, &m_framebuffer);
}

Framebuffer::Framebuffer(GLenum draw_buffer, GLenum read_buffer) {
    glCreateFramebuffers(1, &m_framebuffer);

    glNamedFramebufferDrawBuffer(m_framebuffer, draw_buffer);
    glNamedFramebufferReadBuffer(m_framebuffer, read_buffer);
}

void Framebuffer::bind() {
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
}

void Framebuffer::drawBuffers(int n, const GLenum *bufs) {
    glNamedFramebufferDrawBuffers(m_framebuffer, n, bufs);
}

void Framebuffer::attachTexture(GLuint texture, GLenum attachment) {
    glNamedFramebufferTexture(m_framebuffer, attachment, texture, 0);
}

void Framebuffer::destroy() {
    glDeleteFramebuffers(1, &m_framebuffer);
}
