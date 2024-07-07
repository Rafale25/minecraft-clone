#pragma once

#include <stddef.h>
#include <stdint.h>

// https://www.reddit.com/r/C_Programming/comments/s29903/how_does_endianness_work_for_floats_doubles_and/
static uint32_t load_u32be(const unsigned char *buf)
{
    return (uint32_t)buf[0] << 24 | (uint32_t)buf[1] << 16 |
           (uint32_t)buf[2] <<  8 | (uint32_t)buf[3] <<  0;
}

static float load_floatbe(const unsigned char *buf)
{
    uint32_t i = load_u32be(buf);
    float f;

    f = *(float*)&i;
    // memcpy(&f, &i, 4);

    return f;
}

void putIntBe(uint8_t *buffer, int value)
{
    buffer[0] = (value >> 24) & 0xFF;
    buffer[1] = (value >> 16) & 0xFF;
    buffer[2] = (value >> 8) & 0xFF;
    buffer[3] = (value >> 0) & 0xFF;
}
