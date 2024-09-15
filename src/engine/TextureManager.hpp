#pragma once

#include "glad/gl.h"

#include <string>
#include <unordered_map>

#include "loadTexture.hpp"

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

    GLuint loadTexture(const char* path, int format=GL_RGB, int min_filter=GL_LINEAR, int max_filter=GL_LINEAR, int wrap=GL_REPEAT) {
        const std::string path_string = std::string(path);
        auto it = _textures.find(path_string);

        if (it != _textures.end()) {
            return it->second;
        }

        GLuint texture = createTextureFromPath(path, format, min_filter, max_filter, wrap);
        _textures[path_string] = texture;
        return texture;
    }

private:
    std::unordered_map<std::string, GLuint> _textures;
};
