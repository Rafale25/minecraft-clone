#include "TextureManager.hpp"
#include "loadTexture.hpp"

GLuint TextureManager::loadTexture(const char* path, int32_t format, int32_t min_filter, int32_t max_filter, int32_t wrap) {
    const std::string path_string = std::string(path);
    auto it = m_textures.find(path_string);

    if (it != m_textures.end()) {
        return it->second;
    }

    GLuint texture = createTextureFromPath(path, format, min_filter, max_filter, wrap);
    m_textures[path_string] = texture;
    return texture;
}
