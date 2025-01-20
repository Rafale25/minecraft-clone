#include "Network.hpp"

#include <winsock2.h>
#include <ws2tcpip.h>

static int recvAll(int fd, uint8_t *buffer, size_t size) {
    size_t bytes_received = 0;

    while (1) {
#if defined(_WIN32)
        int recv_size = recv(fd, (char*)(&buffer[bytes_received]), size - bytes_received, 0);
#else
        int recv_size = recv(fd, &buffer[bytes_received], size - bytes_received, 0);
#endif
        bytes_received += recv_size;

        if (recv_size == -1) return -1;
        if (bytes_received == size) break;
    }

    return bytes_received;
}

bool makeNonBlocking(SOCKET sckt) {
    unsigned long non_blocking = 1;
    return (ioctlsocket(sckt, FIONBIO, &non_blocking) == 0);
}

// bool MakeNonBlocking(int fd) {
//     int non_blocking = 1;
//     return (ioctl(fd, FIONBIO, &non_blocking) == 0);

//     /* alternatively:
//     int flags = fcntl(fd, F_GETFL, 0);
//     return (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == 0);
//     */
// }

int NetworkConnection::init() {
    WSADATA wsaData;
    int wsaerr;
    WORD wVersionRequested = MAKEWORD(2, 2);
    wsaerr = WSAStartup(wVersionRequested, &wsaData);

    // Check for initialization success
    if (wsaerr != 0) {
        std::cout << "The Winsock dll not found!" << std::endl;
        return -1;
    } else {
        std::cout << "The Winsock dll found" << std::endl;
        std::cout << "The status: " << wsaData.szSystemStatus << std::endl;
    }

    _socket = INVALID_SOCKET;
    _socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (_socket == INVALID_SOCKET) {
        std::cout << "Error at socket(): " << WSAGetLastError() << std::endl;
        WSACleanup();
        return -1;
    }
    std::cout << "Socket is OK!" << std::endl;

    return 0;
}

int NetworkConnection::connectToServer(const char *ip, int port) {
    // Bind the socket to an IP address and port number
    sockaddr_in service;
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = inet_addr(ip);  // Replace with your desired IP address
    service.sin_port = htons(port);  // Choose a port number

    if (connect(_socket, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR) {
        std::cout << "Client: connect() - Failed to connect: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return -1;
    } else {
        std::cout << "Client: Connect() is OK!" << std::endl;
        std::cout << "Client: Can start sending and receiving data..." << std::endl;
    }

    return 0;
}

int NetworkConnection::poll()
{
    fd_set set;

    while (1)
    {
        FD_ZERO(&set);
        FD_SET(_socket, &set);

        struct timeval tv;
        tv.tv_sec = 3;
        tv.tv_usec = 0;

        tv.tv_sec = 0;
        tv.tv_usec = 10;//timeout; // 50 Microseconds for Polling

        int res = select(_socket + 1, &set, NULL, NULL, &tv);

        if (res == SOCKET_ERROR) {
            std::cout << "Socket Error" << std::endl;
            abort();
            return -1;
        }

        if (res == 0) {
            continue;
        }

        return 0;
    }
}

int NetworkConnection::receive(uint8_t* buffer, uint32_t size) {
#if defined(_WIN32)
    int bytes_read = recv(_socket, (char*)buffer, size, 0);
#else
    int bytes_read = recv(_socket, buffer, size, 0);
#endif
    return bytes_read;
}

void NetworkConnection::receiveAll(uint8_t* buffer, uint32_t size) {
    recvAll(_socket, buffer, size);
}

void NetworkConnection::sendD(const void *data, size_t size) {
#if defined(_WIN32)
   send(_socket, (const char*)data, size, 0);
#else
   send(_socket, data, size, 0);
#endif
}
