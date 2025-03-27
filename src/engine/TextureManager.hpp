#pragma once

#include <string>
#include <unordered_map>

#include <glad/gl.h>

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

    GLuint loadTexture(const char* path, int32_t format=GL_RGB, int32_t min_filter=GL_LINEAR, int32_t max_filter=GL_LINEAR, int32_t wrap=GL_REPEAT);

private:
    std::unordered_map<std::string, GLuint> _textures;
};
