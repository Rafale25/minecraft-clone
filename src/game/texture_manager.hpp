#pragma once

#include <array>
#include <vector>
#include <unordered_map>

#include <glad/gl.h>

#include "enums.hpp"

class TextureManager
{
public:
    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;
    TextureManager(TextureManager&&) = delete;
    TextureManager& operator=(TextureManager&&) = delete;

    static TextureManager& Get()
    {
        static TextureManager _instance;
        return _instance;
    }

    static void loadAllTextures() { Get()._loadAllTextures(); }

private:
    void _loadAllTextures();

    TextureManager() {}

    std::unordered_map<TextureName, const char* const> textures_name = {
        { TextureName::GrassTop,    "grass_block_top.png"},
        { TextureName::GrassSide,   "grass_block_side.png"},
        { TextureName::Dirt,        "dirt.png"},
        { TextureName::Stone,       "stone.png"},
        { TextureName::OakLog,      "oak_log.png"},
        { TextureName::OakLogTop,   "oak_log_top.png"},
        { TextureName::OakLeaves,   "azalea_leaves.png"},
    };

    std::unordered_map<BlockType, std::array<TextureName, 3>> block_textures_path = {
        { BlockType::Grass,     { TextureName::GrassTop,    TextureName::GrassSide, TextureName::Dirt       }},
        { BlockType::Dirt,      { TextureName::Dirt,        TextureName::Dirt,      TextureName::Dirt       }},
        { BlockType::Stone,     { TextureName::Stone,       TextureName::Stone,     TextureName::Stone      }},
        { BlockType::OakLog,    { TextureName::OakLogTop,   TextureName::OakLog,    TextureName::OakLogTop  }},
        { BlockType::OakLeaves, { TextureName::OakLeaves,   TextureName::OakLeaves, TextureName::OakLeaves  }},
    };

public:
    std::unordered_map<BlockType, std::array<GLuint64, 3>> block_textures_handles;
    mutable std::unordered_map<BlockType, std::array<GLuint, 3>> block_textures_ids; // map blocktype to ids
    std::vector<GLuint64> textures_handles; // index is texture id and value is the texture handle
};
