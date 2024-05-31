#include "enums.hpp"

extern const BlockMetadata blocksMetadata[] =
{
    [(int)BlockType::Air]        = {.transparent = true},
    [(int)BlockType::Grass]      = {.transparent = false},
    [(int)BlockType::Dirt]       = {.transparent = false},
    [(int)BlockType::Stone]      = {.transparent = false},
    [(int)BlockType::OakLog]     = {.transparent = false},
    [(int)BlockType::OakLeaves]  = {.transparent = true},
};
