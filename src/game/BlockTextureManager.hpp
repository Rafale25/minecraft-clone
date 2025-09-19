#pragma once

#include "enums.hpp"
#include <array>
#include <vector>
#include <unordered_map>

typedef uint64_t GLuint64;
typedef unsigned int GLuint;

class BlockTextureManager
{
public:
    BlockTextureManager(const BlockTextureManager&) = delete;
    BlockTextureManager& operator=(const BlockTextureManager&) = delete;
    BlockTextureManager(BlockTextureManager&&) = delete;
    BlockTextureManager& operator=(BlockTextureManager&&) = delete;

    static BlockTextureManager& Get()
    {
        static BlockTextureManager _instance;
        return _instance;
    }

    static void loadAllTextures() { Get()._loadAllTextures(); }

private:
    void _loadAllTextures();

    BlockTextureManager() = default;

    std::unordered_map<TextureName, const char* const> textures_name = {
        { TextureName::GrassTop,    "grass_block_top.png"},
        { TextureName::GrassSide,   "grass_block_side.png"},
        { TextureName::Dirt,        "dirt.png"},
        { TextureName::Stone,       "stone.png"},
        { TextureName::OakLog,      "oak_log.png"},
        { TextureName::OakLogTop,   "oak_log_top.png"},
        { TextureName::OakLeaves,   "azalea_leaves.png"},
        { TextureName::Glass,       "glass.png"},
        { TextureName::Water,       "water.png"},
        { TextureName::Sand,        "sand.png"},
        { TextureName::Snow,        "snow.png"},

        { TextureName::OakPlank,        "oak_planks.png"},
        { TextureName::StoneBrick,      "stonebrick.png"},
        { TextureName::Netherrack,      "netherrack.png"},
        { TextureName::Gold,            "gold_block.png"},
        { TextureName::PackedIce,       "ice_packed.png"},
        { TextureName::Lava,            "lava.png"},
        { TextureName::BarrelTop,       "barrel_top.png"},
        { TextureName::BarrelSide,      "barrel_side.png"},
        { TextureName::BarrelBottom,    "barrel_bottom.png"},
        { TextureName::BookshelfTop,    "oak_planks.png"},
        { TextureName::BookshelfSide,   "bookshelf.png"},
        { TextureName::BookshelfBottom, "oak_planks.png"},

        { TextureName::INVALID,     "error.png"},
    };

public:
    std::unordered_map<BlockType, std::array<GLuint64, 6>> block_textures_handles;
    mutable std::unordered_map<BlockType, std::array<GLuint, 6>> block_textures_ids; // map blocktype to ids
    std::vector<GLuint64> textures_handles; // index is texture id and value is the texture handle
};
