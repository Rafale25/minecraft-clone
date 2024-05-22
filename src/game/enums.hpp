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

enum class BlockType : uint8_t {
    Air = 0,
    Grass,
    Dirt,
    Stone,
};

enum class Texture : int {
    GrassTop,
    GrassSide,
    Dirt,
};
