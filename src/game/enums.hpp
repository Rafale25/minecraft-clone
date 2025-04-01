#pragma once

#include <cstdint>

enum Orientation : int32_t {
    Top = 0,
    Bottom = 1,
    Front = 2,
    Back = 3,
    Left = 4,
    Right = 5,
};

// https://stackoverflow.com/questions/45860069/enum-with-struct-as-values
enum class BlockType : uint8_t {
    Air = 0,
    Grass = 1,
    Dirt = 2,
    Stone = 3,
    OakLog = 4,
    OakLeaves = 5,
    Glass = 6,
    Water = 7,
    Sand = 8,
    Snow = 9,
    OakPlank = 10,
    StoneBrick = 11,
    Netherrack = 12,
    Gold = 13,
    PackedIce = 14,
    Lava = 15,
    Barrel = 16,
    Bookshelf = 17,

    INVALID,
};

bool operator==(const BlockType&, const int32_t&);
bool operator>(const BlockType&, const int32_t&);
bool operator<(const BlockType&, const int32_t&);

enum class TextureName : int32_t {
    GrassTop,
    GrassSide,
    Dirt,
    Stone,
    OakLog,
    OakLogTop,
    OakLeaves,
    Glass,
    Water,
    Sand,
    Snow,

    OakPlank,
    StoneBrick,
    Netherrack,
    Gold,
    PackedIce,
    Lava,

    BarrelTop,
    BarrelSide,
    BarrelBottom,

    BookshelfTop,
    BookshelfSide,
    BookshelfBottom,

    INVALID,
};

struct BlockInfo {
    // TODO: Use bit field instead
    bool transparent;
    bool affectsAmbiantOcclusion;
    bool liquid;
    TextureName lz, hz, lx, hx, ly, hy;
};

#define T TextureName

constexpr BlockInfo blocks_info[] = {
    // Air = 0
    { true, false, false, T::INVALID, T::INVALID, T::INVALID, T::INVALID, T::INVALID, T::INVALID },

    // Grass = 1
    { false, true, false, T::GrassSide, T::GrassSide, T::GrassSide, T::GrassSide, T::Dirt, T::GrassTop },

    // Dirt = 2
    { false, true, false, T::Dirt, T::Dirt, T::Dirt, T::Dirt, T::Dirt, T::Dirt },

    // Stone = 3
    { false, true, false, T::Stone, T::Stone, T::Stone, T::Stone, T::Stone, T::Stone },

    // OakLog = 4
    { false, true, false, T::OakLog, T::OakLog, T::OakLog, T::OakLog, T::OakLogTop, T::OakLogTop },

    // OakLeaves = 5
    { true, true, false, T::OakLeaves, T::OakLeaves, T::OakLeaves, T::OakLeaves, T::OakLeaves, T::OakLeaves },

    // Glass = 6
    { true, false, false, T::Glass, T::Glass, T::Glass, T::Glass, T::Glass, T::Glass },

    // Water = 7
    { false, false, true, T::Water, T::Water, T::Water, T::Water, T::Water, T::Water },

    // Sand = 8
    { false, true, false, T::Sand, T::Sand, T::Sand, T::Sand, T::Sand, T::Sand },

    // Snow = 9
    { false, true, false, T::Snow, T::Snow, T::Snow, T::Snow, T::Snow, T::Snow },

    // OakPlank = 10
    { false, true, false, T::OakPlank, T::OakPlank, T::OakPlank, T::OakPlank, T::OakPlank, T::OakPlank },

    // StoneBrick = 11
    { false, true, false, T::StoneBrick, T::StoneBrick, T::StoneBrick, T::StoneBrick, T::StoneBrick, T::StoneBrick },

    // Netherrack = 12
    { false, true, false, T::Netherrack, T::Netherrack, T::Netherrack, T::Netherrack, T::Netherrack, T::Netherrack },

    // Gold = 13
    { false, true, false, T::Gold, T::Gold, T::Gold, T::Gold, T::Gold, T::Gold },

    // PackedIce = 14
    { false, true, false, T::PackedIce, T::PackedIce, T::PackedIce, T::PackedIce, T::PackedIce, T::PackedIce },

    // Lava = 15
    { false, true, false, T::Lava, T::Lava, T::Lava, T::Lava, T::Lava, T::Lava },

    // Barrel = 16
    { false, false, false, T::BarrelSide, T::BarrelSide, T::BarrelSide, T::BarrelSide, T::BarrelBottom, T::BarrelTop },

    // Bookshelf = 17
    { false, false, false, T::BookshelfSide, T::BookshelfSide, T::BookshelfSide, T::BookshelfSide, T::BookshelfBottom, T::BookshelfTop },

    // INVALID
    { false, false, false, T::INVALID, T::INVALID, T::INVALID, T::INVALID, T::INVALID, T::INVALID },
};

#undef T
