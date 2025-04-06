#include "loadTexture.hpp"
#include "Logger.hpp"

GLuint createTextureFromPath(const char *path, int32_t format, int32_t min_filter, int32_t max_filter, int32_t wrap)
{
    GLuint texture;

    glCreateTextures(GL_TEXTURE_2D, 1, &texture);
    glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, min_filter);
    glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, max_filter);
    glTextureParameteri(texture, GL_TEXTURE_WRAP_S, wrap);
    glTextureParameteri(texture, GL_TEXTURE_WRAP_T, wrap);

    GLfloat value;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &value);
    glTextureParameterf(texture, GL_TEXTURE_MAX_ANISOTROPY, value);

    glBindTexture(GL_TEXTURE_2D, texture);

    int32_t width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
    unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (data)
    {
        const int32_t MAX_LEVELS = 4;
        glTextureStorage2D(texture, MAX_LEVELS, GL_RGBA8, width, height);
        glTextureSubImage2D(texture, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, data);
        glGenerateTextureMipmap(texture);
    }
    else
    {
        logE("Failed to load texture: {}", path);
        return -1;
    }
    stbi_image_free(data);

    return texture;
}
