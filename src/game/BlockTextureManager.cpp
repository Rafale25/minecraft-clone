#include "BlockTextureManager.hpp"

#include "TextureManager.hpp"

void BlockTextureManager::_loadAllTextures()
{
    const std::string textures_path = "./assets/textures/";

    for (const auto& [key, value] : block_textures_path) {
        int width, height, channels;
        stbi_info((textures_path + textures_name[value[0]]).c_str(), &width, &height, &channels);

        int format;
        switch (channels)
        {
            case 3:
                format = GL_RGB;
                // format = GL_SRGB;
                break;
            case 4:
                format = GL_RGBA;
                // format = GL_SRGB_ALPHA;
                break;
            default:
                format = GL_RGB;
                // format = GL_SRGB;
                break;
        }

        // format = GL_RGB;

        GLuint texture_top = TextureManager::instance().loadTexture((textures_path + textures_name[value[0]]).c_str(), format, GL_NEAREST_MIPMAP_LINEAR, GL_NEAREST);
        GLuint texture_side = TextureManager::instance().loadTexture((textures_path + textures_name[value[1]]).c_str(), format, GL_NEAREST_MIPMAP_LINEAR, GL_NEAREST);
        GLuint texture_bot = TextureManager::instance().loadTexture((textures_path + textures_name[value[2]]).c_str(), format, GL_NEAREST_MIPMAP_LINEAR, GL_NEAREST);

        // GLuint64 texture_top_handle = 0;
        // GLuint64 texture_side_handle = 0;
        // GLuint64 texture_bot_handle = 0;

        GLuint64 texture_top_handle = glGetTextureHandleARB(texture_top);
        GLuint64 texture_side_handle = glGetTextureHandleARB(texture_side);
        GLuint64 texture_bot_handle = glGetTextureHandleARB(texture_bot);

        if (glIsTextureHandleResidentARB(texture_top_handle)) glMakeTextureHandleResidentARB(texture_top_handle);
        if (glIsTextureHandleResidentARB(texture_side_handle)) glMakeTextureHandleResidentARB(texture_side_handle);
        if (glIsTextureHandleResidentARB(texture_bot_handle)) glMakeTextureHandleResidentARB(texture_bot_handle);

        uint id_base_offset = textures_handles.size();
        uint texture_top_id = id_base_offset + 0;
        uint texture_side_id = id_base_offset + 1;
        uint texture_bot_id = id_base_offset + 2;
        textures_handles.push_back(texture_top_handle);
        textures_handles.push_back(texture_side_handle);
        textures_handles.push_back(texture_bot_handle);

        block_textures_handles.insert( {key, {texture_top_handle, texture_side_handle, texture_bot_handle}} );
        block_textures_ids.insert( {key, {texture_top_id, texture_side_id, texture_bot_id}} );
    }
}
