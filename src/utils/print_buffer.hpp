#pragma once

#include <stdio.h>
#include <cstdint>

inline void printBufferHex(const char *title, const unsigned char *buf, size_t buf_len)
{
    printf("%s [ ", title);
    for (size_t i = 0 ; i < buf_len ; ++i) {
        printf("%02X%s", buf[i], ( i + 1 ) % 16 == 0 ? "\r\n" : " " );
    }
    printf("]\n");
}

inline void printBufferInt(const char *title, const int8_t *buf, size_t buf_len)
{
    printf("%s [ ", title);
    for (size_t i = 0 ; i < buf_len ; ++i) {
        printf("%d ", buf[i]);
    }
    printf("]\n");
}

inline void printBufferUint(const char *title, const uint8_t *buf, size_t buf_len)
{
    printf("%s [ ", title);
    for (size_t i = 0 ; i < buf_len ; ++i) {
        printf("%u ", buf[i]);
    }
    printf("]\n");
}
