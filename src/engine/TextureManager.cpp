#include "TextureManager.hpp"
#include "loadTexture.hpp"

GLuint TextureManager::loadTexture(const char* path, int format, int min_filter, int max_filter, int wrap) {
    const std::string path_string = std::string(path);
    auto it = _textures.find(path_string);

    if (it != _textures.end()) {
        return it->second;
    }

    GLuint texture = createTextureFromPath(path, format, min_filter, max_filter, wrap);
    _textures[path_string] = texture;
    return texture;
}
