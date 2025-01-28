#pragma once

#include "glad/gl.h"

#include <string>
#include <unordered_map>

class TextureManager
{
private:
    TextureManager() = default;

public:
    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;
    TextureManager(TextureManager&&) = delete;
    TextureManager& operator=(TextureManager&&) = delete;

    static TextureManager& instance()
    {
        static TextureManager _instance;
        return _instance;
    }

    GLuint loadTexture(const char* path, int format=GL_RGB, int min_filter=GL_LINEAR, int max_filter=GL_LINEAR, int wrap=GL_REPEAT);

private:
    std::unordered_map<std::string, GLuint> _textures;
};
