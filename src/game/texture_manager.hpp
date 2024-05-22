#pragma once

#include <unordered_map>
#include <glad/gl.h>

#include "chunk.hpp"
#include "enums.hpp"

class TextureManager
{
public:
    void loadAllTextures();

private:
    std::unordered_map<Texture, const char* const> textures_name = {
        { Texture::GrassTop, "grass_block_top.png"},
        { Texture::GrassSide, "grass_block_side.png"},
        { Texture::Dirt, "dirt.png"},
    };

    std::unordered_map<BlockType, std::array<Texture, 3>> block_textures_path = {
        { BlockType::Grass, { Texture::GrassTop, Texture::GrassSide, Texture::Dirt }},
        { BlockType::Dirt, { Texture::Dirt, Texture::Dirt, Texture::Dirt } }
    };

public:
    std::unordered_map<BlockType, std::array<GLuint64, 3>> block_textures_handles;
};
