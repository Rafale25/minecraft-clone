#pragma once

#include <cstdint>

inline void putIntBe(uint8_t *buffer, int value)
{
    buffer[0] = (value >> 24) & 0xFF;
    buffer[1] = (value >> 16) & 0xFF;
    buffer[2] = (value >> 8) & 0xFF;
    buffer[3] = (value >> 0) & 0xFF;
}
