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
    std::unordered_map<TextureName, const char* const> textures_name = {
        { TextureName::GrassTop, "grass_block_top.png"},
        { TextureName::GrassSide, "grass_block_side.png"},
        { TextureName::Dirt, "dirt.png"},
        { TextureName::Stone, "stone.png"},
        { TextureName::OakLog, "oak_log.png"},
        { TextureName::OakLogTop, "oak_log_top.png"},
        { TextureName::OakLeaves, "azalea_leaves.png"},
    };

    std::unordered_map<BlockType, std::array<TextureName, 3>> block_textures_path = {
        { BlockType::Grass, { TextureName::GrassTop, TextureName::GrassSide, TextureName::Dirt }},
        { BlockType::Dirt, { TextureName::Dirt, TextureName::Dirt, TextureName::Dirt } },
        { BlockType::Stone, { TextureName::Stone, TextureName::Stone, TextureName::Stone } },
        { BlockType::OakLog, { TextureName::OakLogTop, TextureName::OakLog, TextureName::OakLogTop } },
        { BlockType::OakLeaves, { TextureName::OakLeaves, TextureName::OakLeaves, TextureName::OakLeaves } },
    };

public:
    std::unordered_map<BlockType, std::array<GLuint64, 3>> block_textures_handles;
};
