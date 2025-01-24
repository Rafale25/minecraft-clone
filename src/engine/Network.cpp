#include "Network.hpp"

#if defined(_WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <sys/select.h>
#endif

static int recvAll(int fd, uint8_t *buffer, size_t size) {
    size_t bytes_received = 0;

    while (1) {
#if defined(_WIN32)
        int recv_size = recv(fd, (char*)(&buffer[bytes_received]), size - bytes_received, 0);
#else
        int recv_size = recv(fd, &(buffer[bytes_received]), size - bytes_received, 0);
#endif
        bytes_received += recv_size;

        if (recv_size == -1) return -1;
        if (bytes_received == size) break;
    }

    return bytes_received;
}

#if defined(_WIN32)
bool setNonBlocking(SOCKET sckt) {
    unsigned long non_blocking = 1;
    return (ioctlsocket(sckt, FIONBIO, &non_blocking) == 0);
}
#else
bool setNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    return (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == 0);
}
#endif


int NetworkConnection::init() {
#if defined(_WIN32)
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
#else
    _socket = socket(AF_INET, SOCK_STREAM, 0);
    return 0;
#endif
}

int NetworkConnection::connectToServer(const char *ip, int port) {
#if defined(_WIN32)
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
#else
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(ip);
    serverAddress.sin_port = htons(port);

    struct timeval tv;
    tv.tv_sec = 3;
    tv.tv_usec = 0;

    fd_set set;
    FD_ZERO(&set);
    FD_SET(_socket, &set);

    // Set to blocking mode
    int opts = fcntl(_socket, F_SETFL, O_NONBLOCK); // https://stackoverflow.com/questions/2597608/c-socket-connection-timeout

    // setNonBlocking(_socket);

    printf("Connecting to %s...\n", ip);
    int res = connect(_socket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
    if (errno != EINPROGRESS) {
        printf("Connection failed.\n");
        return -1;
    }

    res = select(_socket+1, NULL, &set, NULL, &tv);

    if (res < 0 && errno != EINTR) {
        printf("Error connecting %d - %s\n", errno, strerror(errno));
        exit(0);
    } else if (res > 0) {

        socklen_t lon = sizeof(int);
        int valopt;

        if (getsockopt(_socket, SOL_SOCKET, SO_ERROR, (void*)(&valopt), &lon) < 0) {
            fprintf(stderr, "Error in getsockopt() %d - %s\n", errno, strerror(errno));
            exit(0);
        }
        if (valopt) { // Check the value returned...
            fprintf(stderr, "Error in delayed connection() %d - %s\n", valopt, strerror(valopt));
            exit(0);
        }
        printf("Successfully connected to %s\n", ip);

    } else {
        printf("Connection timeout.\n");
        exit(0);
    }

    // Set blocking mode back
    opts = opts & (~O_NONBLOCK);
    fcntl(_socket, F_SETFL, opts);

    return 0;
#endif
}

int NetworkConnection::waitForData(const bool& should_stop)
{
#if defined(_WIN32)
    fd_set set;
    while (!should_stop)
    {
        FD_ZERO(&set);
        FD_SET(_socket, &set);

        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 30; //timeout; // N Microseconds for Polling

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
    return 0;
#else
    struct pollfd fds;
    fds.fd = _socket;
    fds.events = POLLIN;

    while (!should_stop) {
        constexpr int timeout = 8;

        // TODO URGENT PROBLEM: code can get infinitly stuck here since there's nothing stopping this loop if we stop the game and the server doesn't send anything
        int rv = poll(&fds, 1, timeout); // poll (check if server sent anything)
        if ((rv > 0 && (fds.revents & POLLIN))) break;
    }
    return 0;
#endif
}

int NetworkConnection::receive(uint8_t* buffer, uint32_t size) {
#if defined(_WIN32)
    int bytes_read = recv(_socket, (char*)buffer, size, 0);
#else
    int bytes_read = recv(_socket, buffer, size, 0);
#endif
    return bytes_read;
}

int NetworkConnection::receiveAll(uint8_t* buffer, uint32_t size) {
    return recvAll(_socket, buffer, size);
}

void NetworkConnection::sendD(const void *data, size_t size) {
#if defined(_WIN32)
   send(_socket, (const char*)data, size, 0);
#else
   send(_socket, data, size, 0);
#endif
}

#if defined(_WIN32)
void NetworkConnection::closeConnection() {
    closesocket(_socket);
}
#else
#include <unistd.h>
void NetworkConnection::closeConnection() {
    close(_socket);
}
#endif
