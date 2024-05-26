#include <iostream>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>

#include "client.hpp"
#include "chunk.hpp"
#include "world.hpp"
#include "entity.hpp"

#include <cstring>
#include <endian.h>

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

Chunk readChunk(uint8_t *buffer)
{
    uint8_t *head = &buffer[0];
    int x, y, z;

    // skip packet id
    // head += sizeof(uint8_t);

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

Client::Client(World &world):
    world(&world)
{
    client_socket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    // serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddress.sin_addr.s_addr = inet_addr("51.77.194.124");
    serverAddress.sin_port = htons(15000);

    connect(client_socket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
}

void Client::Start()
{
    client_thread = std::thread(&Client::client_thread_func, this);
}

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

std::tuple<int, glm::vec3> readAddEntityPacket(uint8_t *buffer)
{
    uint8_t *head = &buffer[0];
    int entityId;
    float x, y, z;

    // skip packet id
    // head += sizeof(uint8_t);

    entityId = *(int*)&head[0];
    head += sizeof(int);

    x = load_floatbe(&head[0]);
    head += sizeof(float);

    y = load_floatbe(&head[0]);
    head += sizeof(float);

    z = load_floatbe(&head[0]);
    head += sizeof(float);

    return std::tuple<int, glm::vec3>({htonl(entityId), glm::vec3(x, y, z)});
}

std::tuple<int, glm::vec3, float, float> readUpdateEntityPacket(uint8_t *buffer)
{
    uint8_t *head = &buffer[0];
    int entityId;
    float x, y, z;
    float yaw, pitch;

    // skip packet id
    // head += sizeof(uint8_t);

    entityId = *(int*)&head[0];
    head += sizeof(int);

    x = load_floatbe(&head[0]);
    head += sizeof(float);

    y = load_floatbe(&head[0]);
    head += sizeof(float);

    z = load_floatbe(&head[0]);
    head += sizeof(float);

    yaw = load_floatbe(&head[0]);
    head += sizeof(float);

    pitch = load_floatbe(&head[0]);
    head += sizeof(float);

    return std::tuple<int, glm::vec3, float, float>({htonl(entityId), glm::vec3(x, y, z), yaw, pitch});
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

            int recv_size = recv(client_socket, buffer, 1, 0);
            // int recv_size = recv_full(client_socket, buffer, 5000);
            if (recv_size == -1) {
                std::cout << "recv failed: return -1" << std::endl;
                break;
            }

            uint8_t id = buffer[0];

            /*
            sizes int bytes without id
            0x00 4
            0x01 24
            0x02 4
            0x03 24
            0x04 4108
            */

            switch (id)
            {
                case 0x00: // identification
                    recv_full(client_socket, buffer, 4);
                    {
                        client_id = be32toh(*(int*)(&buffer[0]));
                        printf("Client id: %d\n", client_id);
                    }
                    break;
                case 0x01: // add entity
                    recv_full(client_socket, buffer, 24);
                    {
                        auto [id, pos] = readAddEntityPacket(buffer);
                        printf("Server sent 'add entity' packet: %d %f %f %f.\n", id, pos.x, pos.y, pos.z);

                        task_queue_mutex.lock();
                        task_queue.push_front([this, id, pos]() {
                            Entity e{id};
                            e.transform.position = pos;
                            world->add_entity(e);
                        } );
                        task_queue_mutex.unlock();
                    }
                    break;
                case 0x02: // remove entity
                    recv_full(client_socket, buffer, 4);
                    {
                        printf("Server sent 'remove entity' packet: %d.\n", id);
                        task_queue_mutex.lock();
                        task_queue.push_front([this, id]() {
                            world->remove_entity(id);
                        } );
                        task_queue_mutex.unlock();
                    }
                    break;
                case 0x03: // update entity
                    recv_full(client_socket, buffer, 24);
                    {
                        auto [id, pos, yaw, pitch] = readUpdateEntityPacket(buffer);
                        printf("Server sent 'move entity' packet: %d %f %f %f.\n", id, pos.x, pos.y, pos.z);
                        task_queue_mutex.lock();
                        task_queue.push_front([this, id, pos, yaw, pitch]() {
                            world->update_entity(id, pos, yaw, pitch);
                        } );
                        task_queue_mutex.unlock();
                    }
                    break;
                case 0x04: // Send Chunk
                    recv_full(client_socket, buffer, 4108);
                    {
                        world->chunks_mutex.lock();
                        Chunk c = readChunk(buffer);
                        world->chunks[c.pos] = c;
                        world->chunks_mutex.unlock();

                        printf("Server sent 'chunk' packet: %d %d %d.\n", id, c.pos.x, c.pos.y, c.pos.z);
                    }
                    break;
                default:
                    break;
            }

        }

        // check queue for info to send to server

    }
}
