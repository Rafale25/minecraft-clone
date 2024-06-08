#include "texture.hpp"

Texture::Texture(GLsizei width, GLsizei height, GLenum format, GLenum min_filter, GLenum mag_filter, GLenum wrap) {
    _width = width;
    _height = height;

    glCreateTextures(GL_TEXTURE_2D, 1, &_texture);

    glTextureParameteri(_texture, GL_TEXTURE_MIN_FILTER, min_filter);
    glTextureParameteri(_texture, GL_TEXTURE_MAG_FILTER, mag_filter);
    glTextureParameteri(_texture, GL_TEXTURE_WRAP_S, wrap);
    glTextureParameteri(_texture, GL_TEXTURE_WRAP_T, wrap);

    glTextureStorage2D(_texture, 1, format, width, height);
}
Texture::Texture(GLsizei width, GLsizei height, GLenum format, GLenum min_filter, GLenum mag_filter, GLenum wrap, const float borderColor[4]) {
    // create(width, height, format, min_filter, mag_filter, wrap);
    _width = width;
    _height = height;

    glCreateTextures(GL_TEXTURE_2D, 1, &_texture);

    glTextureParameteri(_texture, GL_TEXTURE_MIN_FILTER, min_filter);
    glTextureParameteri(_texture, GL_TEXTURE_MAG_FILTER, mag_filter);
    glTextureParameteri(_texture, GL_TEXTURE_WRAP_S, wrap);
    glTextureParameteri(_texture, GL_TEXTURE_WRAP_T, wrap);
    glTextureParameterfv(_texture, GL_TEXTURE_BORDER_COLOR, borderColor);

    glTextureStorage2D(_texture, 1, format, width, height);
}
