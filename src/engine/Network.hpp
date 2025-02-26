#pragma once

#include <cstdint>

#if defined(_WIN32)
#include <basetsd.h>
typedef UINT_PTR SOCKET;
#endif

class NetworkConnection {
public:
    NetworkConnection() = default;

    int init();
    int connectToServer(const char* ip, int port);
    int waitForData(const bool& should_stop);
    int receive(uint8_t* buffer, uint32_t size);
    int receiveAll(uint8_t* buffer, uint32_t size);
    void sendD(const void *data, uint32_t size);
    void closeConnection();

private:

#if defined(_WIN32)
    SOCKET _socket;
#else
    int _socket;
#endif
};
