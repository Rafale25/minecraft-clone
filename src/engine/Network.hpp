#pragma once

#include <iostream>

#include <cstdint>

#if defined(_WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

class NetworkConnection {
public:
    NetworkConnection(int timeout): timeout(timeout) {};

    int init();
    int connectToServer(const char* ip, int port);
    int waitForData();
    int receive(uint8_t* buffer, uint32_t n);
    void receiveAll(uint8_t* buffer, uint32_t size);
    void sendD(const void *data, size_t size);

private:

#if defined(_WIN32)
    SOCKET _socket;
#else
    int _socket;
#endif

    int timeout;
};
