#pragma once

#include <cstdint>

enum Orientation : int {
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
    LAST, // do not use as block
};

bool operator==(const BlockType&, const int&);
bool operator>(const BlockType&, const int&);
bool operator<(const BlockType&, const int&);

struct BlockMetadata
{
    bool transparent;
    // bool liquid;
    // ...
};

// extern const BlockMetadata blocksMetadata[];

inline constexpr BlockMetadata blocksMetadata[] =
{
    [(int)BlockType::Air]        = {.transparent = true},
    [(int)BlockType::Grass]      = {.transparent = false},
    [(int)BlockType::Dirt]       = {.transparent = false},
    [(int)BlockType::Stone]      = {.transparent = false},
    [(int)BlockType::OakLog]     = {.transparent = false},
    [(int)BlockType::OakLeaves]  = {.transparent = true},

    [(int)BlockType::Glass]  = {.transparent = true},
    [(int)BlockType::Water]  = {.transparent = false},
    [(int)BlockType::Sand]  = {.transparent = false},
    [(int)BlockType::Snow]  = {.transparent = false},
};


enum class TextureName : int {
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
};

enum PacketId {
    IDENTIFICATION = 0x00,
    ADD_ENTITY = 0x01,
    REMOVE_ENTITY = 0x02,
    UPDATE_ENTITY = 0x03,
    CHUNK = 0x04,
    MONOTYPE_CHUNK = 0x05,
    CHAT_MESSAGE = 0x06,
    UPDATE_ENTITY_METADATA = 0x07
};
