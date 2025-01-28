#include "loadTexture.hpp"

#include <iostream>

GLuint createTextureFromPath(const char *path, int format, int min_filter, int max_filter, int wrap)
{
    GLuint texture;

    glCreateTextures(GL_TEXTURE_2D, 1, &texture);
    glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, min_filter);
    glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, max_filter);
    glTextureParameteri(texture, GL_TEXTURE_WRAP_S, wrap);
    glTextureParameteri(texture, GL_TEXTURE_WRAP_T, wrap);

    glBindTexture(GL_TEXTURE_2D, texture);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
    unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (data)
    {
        const int MAX_LEVELS = 4;
        glTextureStorage2D(texture, MAX_LEVELS, GL_RGBA8, width, height);
        glTextureSubImage2D(texture, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, data);
        glGenerateTextureMipmap(texture);
    }
    else
    {
        std::cout << "Failed to load texture: " << path << std::endl;
        return -1;
    }
    stbi_image_free(data);

    return texture;
}
