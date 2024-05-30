#pragma once

#include <array>
#include <unordered_map>
#include <glad/gl.h>

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
        { Texture::Stone, "stone.png"},
        { Texture::OakLog, "oak_log.png"},
        { Texture::OakLogTop, "oak_log_top.png"},
        { Texture::OakLeaves, "azalea_leaves.png"},
    };

    std::unordered_map<BlockType, std::array<Texture, 3>> block_textures_path = {
        { BlockType::Grass, { Texture::GrassTop, Texture::GrassSide, Texture::Dirt }},
        { BlockType::Dirt, { Texture::Dirt, Texture::Dirt, Texture::Dirt } },
        { BlockType::Stone, { Texture::Stone, Texture::Stone, Texture::Stone } },
        { BlockType::OakLog, { Texture::OakLogTop, Texture::OakLog, Texture::OakLogTop } },
        { BlockType::OakLeaves, { Texture::OakLeaves, Texture::OakLeaves, Texture::OakLeaves } },
    };

public:
    std::unordered_map<BlockType, std::array<GLuint64, 3>> block_textures_handles;
};
