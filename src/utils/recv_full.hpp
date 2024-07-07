#pragma once

#include <stddef.h>
#include <stdint.h>
#include <sys/socket.h>

int recv_full(int fd, uint8_t *buffer, size_t size)
{
    size_t bytes_received = 0;

    while (1) {
        int recv_size = recv(fd, &buffer[bytes_received], size - bytes_received, 0);
        bytes_received += recv_size;

        if (recv_size == -1)
            return -1;

        if (bytes_received == size)
            break;
    }

    return bytes_received;
}
