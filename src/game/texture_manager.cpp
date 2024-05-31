#include "texture_manager.hpp"
#include "texture.hpp"

void TextureManager::loadAllTextures()
{
    std::string textures_path = "assets/textures/";

    for (const auto& [key, value] : block_textures_path) {
        // TODO: make texture manager to avoid duplicated when calling loadTextures

        int width, height, channels;
        stbi_info((textures_path + textures_name[value[0]]).c_str(), &width, &height, &channels);

        int format;
        switch (channels)
        {
            case 3:
                format = GL_RGB;
                break;
            case 4:
                format = GL_RGBA;
                break;
            default:
                format = GL_RGB;
                break;
        }

        GLuint texture_top = loadTexture((textures_path + textures_name[value[0]]).c_str(), format, GL_NEAREST_MIPMAP_LINEAR, GL_NEAREST);
        GLuint texture_side = loadTexture((textures_path + textures_name[value[1]]).c_str(), format, GL_NEAREST_MIPMAP_LINEAR, GL_NEAREST);
        GLuint texture_bot = loadTexture((textures_path + textures_name[value[2]]).c_str(), format, GL_NEAREST_MIPMAP_LINEAR, GL_NEAREST);

        GLuint64 texture_top_handle = glGetTextureHandleARB(texture_top);
        GLuint64 texture_side_handle = glGetTextureHandleARB(texture_side);
        GLuint64 texture_bot_handle = glGetTextureHandleARB(texture_bot);

        glMakeTextureHandleResidentARB(texture_top_handle);
        glMakeTextureHandleResidentARB(texture_side_handle);
        glMakeTextureHandleResidentARB(texture_bot_handle);

        block_textures_handles.insert( {key, {texture_top_handle, texture_side_handle, texture_bot_handle}} );
    }
}
