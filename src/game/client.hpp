#pragma once

#include <stdio.h>
#include <unordered_map>
#include <thread>
#include <mutex>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "chunk.hpp"


class Client
{
public:
    Client(std::unordered_map<glm::ivec3, Chunk> &chunks, std::mutex &chunks_mutex):
        chunks(&chunks), chunks_mutex(&chunks_mutex)
    {
        clientSocket = socket(AF_INET, SOCK_STREAM, 0);

        sockaddr_in serverAddress;
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
        serverAddress.sin_port = htons(15000);

        connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

        uint8_t buffer[5000] = {0};
        recv(clientSocket, buffer, sizeof(buffer), 0);
        client_id = buffer[0];
        printf("%d\n", client_id);
    }

    void Start()
    {
        client_thread = std::thread(&Client::client_thread_func, this);
    }

    void client_thread_func()
    {
        while (true)
        {
            // if ((rv = poll(fds, 1, 100) > 0)) {
            // }

            // poll (check if server sent anything)

            // check queue for info to send to server

            // mutex.lock();
            // chunks;
            // mutex.unlock();
        }
    }

private:
    int clientSocket;
    int client_id;

    std::thread client_thread;

    std::unordered_map<glm::ivec3, Chunk> *chunks;
    std::mutex *chunks_mutex;
};
