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
    LAST, // do not use as block
};

struct BlockMetadata
{
    bool transparent;
    // ...
};

extern const BlockMetadata blocksMetadata[];

enum class Texture : int {
    GrassTop,
    GrassSide,
    Dirt,
    Stone,
    OakLog,
    OakLogTop,
    OakLeaves
};
