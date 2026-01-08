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

#define BLOCKTYPES \
    X(Air, 0) \
    X(Grass, 1) \
    X(Dirt, 2) \
    X(Stone, 3) \
    X(OakLog, 4) \
    X(OakLeaves, 5) \
    X(Glass, 6) \
    X(Water, 7) \
    X(Sand, 8) \
    X(Snow, 9) \
    X(OakPlank, 10) \
    X(StoneBrick, 11) \
    X(Netherrack, 12) \
    X(Gold, 13) \
    X(PackedIce, 14) \
    X(Lava, 15) \
    X(Barrel, 16) \
    X(Bookshelf, 17) \
    X(INVALID, 18)

// https://stackoverflow.com/questions/45860069/enum-with-struct-as-values
enum class BlockType : uint8_t {
    #define X(name, value) name = value,
    BLOCKTYPES
    #undef X
};

inline constexpr BlockType g_blocktypes[] = {
    #define X(name, value) BlockType::name,
    BLOCKTYPES
    #undef X
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

struct TextureCube {
    TextureName lz, hz, lx, hx, ly, hy;
};

struct BlockInfo {
    // TODO: Use bit field instead
    bool transparent;
    bool translucent;
    bool affectsAmbiantOcclusion;
    bool liquid;
    TextureCube textures;
};

#define T TextureName

constexpr BlockInfo g_blocksInfo[] = {
    // Air = 0
    { true, false, false, false, {T::INVALID, T::INVALID, T::INVALID, T::INVALID, T::INVALID, T::INVALID} },

    // Grass = 1
    { false, false, true, false, {T::GrassSide, T::GrassSide, T::GrassSide, T::GrassSide, T::Dirt, T::GrassTop} },

    // Dirt = 2
    { false, false, true, false, {T::Dirt, T::Dirt, T::Dirt, T::Dirt, T::Dirt, T::Dirt} },

    // Stone = 3
    { false, false, true, false, {T::Stone, T::Stone, T::Stone, T::Stone, T::Stone, T::Stone} },

    // OakLog = 4
    { false, false, true, false, {T::OakLog, T::OakLog, T::OakLog, T::OakLog, T::OakLogTop, T::OakLogTop} },

    // OakLeaves = 5
    { true, false, true, false, {T::OakLeaves, T::OakLeaves, T::OakLeaves, T::OakLeaves, T::OakLeaves, T::OakLeaves} },

    // Glass = 6
    { true, false, false, false, {T::Glass, T::Glass, T::Glass, T::Glass, T::Glass, T::Glass} },

    // Water = 7
    { false, true, false, true, {T::Water, T::Water, T::Water, T::Water, T::Water, T::Water} },

    // Sand = 8
    { false, false, true, false, {T::Sand, T::Sand, T::Sand, T::Sand, T::Sand, T::Sand} },

    // Snow = 9
    { false, false, true, false, {T::Snow, T::Snow, T::Snow, T::Snow, T::Snow, T::Snow} },

    // OakPlank = 10
    { false, false, true, false, {T::OakPlank, T::OakPlank, T::OakPlank, T::OakPlank, T::OakPlank, T::OakPlank} },

    // StoneBrick = 11
    { false, false, true, false, {T::StoneBrick, T::StoneBrick, T::StoneBrick, T::StoneBrick, T::StoneBrick, T::StoneBrick} },

    // Netherrack = 12
    { false, false, true, false, {T::Netherrack, T::Netherrack, T::Netherrack, T::Netherrack, T::Netherrack, T::Netherrack} },

    // Gold = 13
    { false, false, true, false, {T::Gold, T::Gold, T::Gold, T::Gold, T::Gold, T::Gold} },

    // PackedIce = 14
    { false, false, true, false, {T::PackedIce, T::PackedIce, T::PackedIce, T::PackedIce, T::PackedIce, T::PackedIce} },

    // Lava = 15
    { false, false, true, false, {T::Lava, T::Lava, T::Lava, T::Lava, T::Lava, T::Lava} },

    // Barrel = 16
    { false, false, true, false, {T::BarrelSide, T::BarrelSide, T::BarrelSide, T::BarrelSide, T::BarrelBottom, T::BarrelTop} },

    // Bookshelf = 17
    { false, false, true, false, {T::BookshelfSide, T::BookshelfSide, T::BookshelfSide, T::BookshelfSide, T::BookshelfBottom, T::BookshelfTop} },

    // INVALID
    { false, false, false, false, {T::INVALID, T::INVALID, T::INVALID, T::INVALID, T::INVALID, T::INVALID} },
};

#undef T
