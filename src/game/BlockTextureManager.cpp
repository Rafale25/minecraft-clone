#include "BlockTextureManager.hpp"

#include "TextureManager.hpp"
#include "stb_image.h"

// #define DISABLE_BINDLESS_TEXTURE

static int32_t getFormat(const char* filepath)
{
    int32_t width, height, channels;
    stbi_info(filepath, &width, &height, &channels);

    int32_t format;
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

    return format;
}

void BlockTextureManager::_loadAllTextures()
{
    const std::string textures_path = "./assets/textures/";

    for (int32_t i = 0 ; i < (int32_t)BlockType::INVALID ; ++i) {
        const auto& [transparent, liquid, lz, hz, lx, hx, ly, hy] = blocks_info[i];

        std::string filepath_lz = (textures_path + textures_name[lz]);
        std::string filepath_hz = (textures_path + textures_name[hz]);
        std::string filepath_lx = (textures_path + textures_name[lx]);
        std::string filepath_hx = (textures_path + textures_name[hx]);
        std::string filepath_ly = (textures_path + textures_name[ly]);
        std::string filepath_hy = (textures_path + textures_name[hy]);

        GLuint texture_lz = TextureManager::instance().loadTexture(filepath_lz.c_str(), getFormat(filepath_lz.c_str()), GL_NEAREST_MIPMAP_LINEAR, GL_NEAREST);
        GLuint texture_hz = TextureManager::instance().loadTexture(filepath_hz.c_str(), getFormat(filepath_hz.c_str()), GL_NEAREST_MIPMAP_LINEAR, GL_NEAREST);
        GLuint texture_lx = TextureManager::instance().loadTexture(filepath_lx.c_str(), getFormat(filepath_lx.c_str()), GL_NEAREST_MIPMAP_LINEAR, GL_NEAREST);
        GLuint texture_hx = TextureManager::instance().loadTexture(filepath_hx.c_str(), getFormat(filepath_hx.c_str()), GL_NEAREST_MIPMAP_LINEAR, GL_NEAREST);
        GLuint texture_ly = TextureManager::instance().loadTexture(filepath_ly.c_str(), getFormat(filepath_ly.c_str()), GL_NEAREST_MIPMAP_LINEAR, GL_NEAREST);
        GLuint texture_hy = TextureManager::instance().loadTexture(filepath_hy.c_str(), getFormat(filepath_hy.c_str()), GL_NEAREST_MIPMAP_LINEAR, GL_NEAREST);

#ifdef DISABLE_BINDLESS_TEXTURE
        GLuint64 texture_handle_lz = 0;
        GLuint64 texture_handle_hz = 0;
        GLuint64 texture_handle_lx = 0;
        GLuint64 texture_handle_hx = 0;
        GLuint64 texture_handle_ly = 0;
        GLuint64 texture_handle_hy = 0;
#else
        GLuint64 texture_handle_lz = glGetTextureHandleARB(texture_lz);
        GLuint64 texture_handle_hz = glGetTextureHandleARB(texture_hz);
        GLuint64 texture_handle_lx = glGetTextureHandleARB(texture_lx);
        GLuint64 texture_handle_hx = glGetTextureHandleARB(texture_hx);
        GLuint64 texture_handle_ly = glGetTextureHandleARB(texture_ly);
        GLuint64 texture_handle_hy = glGetTextureHandleARB(texture_hy);

        if (!glIsTextureHandleResidentARB(texture_handle_lz)) glMakeTextureHandleResidentARB(texture_handle_lz);
        if (!glIsTextureHandleResidentARB(texture_handle_hz)) glMakeTextureHandleResidentARB(texture_handle_hz);
        if (!glIsTextureHandleResidentARB(texture_handle_lx)) glMakeTextureHandleResidentARB(texture_handle_lx);
        if (!glIsTextureHandleResidentARB(texture_handle_hx)) glMakeTextureHandleResidentARB(texture_handle_hx);
        if (!glIsTextureHandleResidentARB(texture_handle_ly)) glMakeTextureHandleResidentARB(texture_handle_ly);
        if (!glIsTextureHandleResidentARB(texture_handle_hy)) glMakeTextureHandleResidentARB(texture_handle_hy);
#endif

        uint32_t id_base_offset = textures_handles.size();
        uint32_t texture_id_lz = id_base_offset + 0;
        uint32_t texture_id_hz = id_base_offset + 1;
        uint32_t texture_id_lx = id_base_offset + 2;
        uint32_t texture_id_hx = id_base_offset + 3;
        uint32_t texture_id_ly = id_base_offset + 4;
        uint32_t texture_id_hy = id_base_offset + 5;

        textures_handles.push_back(texture_handle_lz);
        textures_handles.push_back(texture_handle_hz);
        textures_handles.push_back(texture_handle_lx);
        textures_handles.push_back(texture_handle_hx);
        textures_handles.push_back(texture_handle_ly);
        textures_handles.push_back(texture_handle_hy);

        block_textures_handles.insert( {(BlockType)i, {texture_handle_lz, texture_handle_hz, texture_handle_lx, texture_handle_hx, texture_handle_ly, texture_handle_hy}} );
        block_textures_ids.insert( {(BlockType)i, {texture_id_lz, texture_id_hz, texture_id_lx, texture_id_hx, texture_id_ly, texture_id_hy}} );
    }
}
