#include <iostream>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>

#include "client.hpp"
#include "chunk.hpp"

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

    return 0;
}

Chunk readChunk(uint8_t *buffer)
{
    uint8_t *head = &buffer[0];
    int x, y, z;

    // skip packet id
    head += sizeof(uint8_t);

    x = *(int*)&head[0];
    head += sizeof(int);

    y = *(int*)&head[0];
    head += sizeof(int);

    z = *(int*)&head[0];
    head += sizeof(int);

    Chunk chunk;
    chunk.pos = glm::ivec3(htonl(x), htonl(y), htonl(z)) / 16;

    glm::vec3 chunkPosWorld = chunk.pos * 16;

    for (int i = 0 ; i < 16*16*16 ; ++i) {
        uint8_t byte = *(uint8_t*)&head[0];
        head += sizeof(uint8_t);

        /* convert to BlackoutBurst indexing -_- */
        int x = i % 16;
        int y = (i / 16) % 16;
        int z = i / (16 * 16);
        int index = x * 16*16 + y * 16 + z;

        chunk.blocks[index] = (BlockType)byte;
    }

    // chunk.computeChunckVAO(texture_manager);

    return chunk;
}

Client::Client(std::unordered_map<glm::ivec3, Chunk> *chunks, std::mutex &chunks_mutex):
    chunks(chunks), chunks_mutex(&chunks_mutex)
{
    client_socket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddress.sin_port = htons(15000);

    connect(client_socket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
}

void Client::Start()
{
    client_thread = std::thread(&Client::client_thread_func, this);
}

void Client::client_thread_func()
{
    struct pollfd fds;
    fds.fd = client_socket;
    fds.events = POLLIN;

    uint8_t buffer[5000] = {0};

    while (true)
    {
        // poll (check if server sent anything)
        int rv = poll(&fds, 1, 0);
        if (rv > 0 && (fds.revents & POLLIN)) {
            int res = recv_full(client_socket, buffer, 5000);
            if (res == -1) {
                std::cout << "recv failed: return -1" << std::endl;
                break;
            }

            uint8_t id = buffer[0];

            switch (id)
            {
                case 0x00: // move entity
                    break;
                case 0x01: // add entity
                    break;
                case 0x02: // remove entity
                    break;
                case 0x03: // identification
                    {
                        client_id = buffer[0];
                        printf("Client id: %d\n", client_id);
                    }
                    break;
                case 0x04: // Entity Rotation
                    break;
                case 0x05: // Send Chunk
                    {
                        chunks_mutex->lock();
                        Chunk c = readChunk(buffer);
                        (*chunks)[c.pos] = c;
                        chunks_mutex->unlock();
                    }
                    break;
                default:
                    break;
            }

        }

        // check queue for info to send to server

    }
}
