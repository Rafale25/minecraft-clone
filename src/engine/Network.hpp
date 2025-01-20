#pragma once

#include <iostream>

#include <cstdint>
#include <winsock2.h>
#include <ws2tcpip.h>

class NetworkConnection {
public:
    NetworkConnection(int timeout): timeout(timeout) {};

    int init();
    int connectToServer(const char* ip, int port);
    int poll();
    int receive(uint8_t* buffer, uint32_t n);
    void receiveAll(uint8_t* buffer, uint32_t size);
    void sendD(const void *data, size_t size);

private:
    SOCKET _socket;
    int timeout;
};
