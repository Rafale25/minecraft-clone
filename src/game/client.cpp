#include <iostream>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <fcntl.h>
#include <sys/select.h>
#include <cstring>
#include <endian.h>

#include "client.hpp"
#include "chunk.hpp"
#include "world.hpp"
#include "entity.hpp"
#include "utils/print_buffer.hpp"

#include "utils/recv_full.hpp"
#include "utils/byte_manipulation.hpp"

#include "ByteBuffer.hpp"

ChunkPacket* readChunkPacket(ByteBuffer buffer)
{
    ChunkPacket* chunk_data = new ChunkPacket;

    int x = buffer.getInt();
    int y = buffer.getInt();
    int z = buffer.getInt();

    chunk_data->pos = glm::ivec3(x, y, z) / 16;

    for (int i = 0 ; i < 16*16*16 ; ++i) {
        uint8_t byte = buffer.get();

        /* convert to BlackoutBurst indexing -_- */
        // int x = i % 16;
        // int y = (i / 16) % 16;
        // int z = i / (16 * 16);
        // int index = x * 16*16 + y * 16 + z;
        // chunk.blocks[index] = (BlockType)byte;

        chunk_data->blocks[i] = (BlockType)byte;
    }

    return chunk_data;
}

ChunkPacket* readFullMonoChunkPacket(ByteBuffer buffer)
{
    ChunkPacket *chunk_data = new ChunkPacket;

    int x = buffer.getInt();
    int y = buffer.getInt();
    int z = buffer.getInt();
    uint8_t blockType = buffer.get();

    chunk_data->pos = glm::ivec3(x, y, z) / 16;
    memset(chunk_data->blocks, blockType, 16*16*16);

    return chunk_data;
}

AddEntityClientPacket readAddEntityPacket(ByteBuffer buffer)
{
    AddEntityClientPacket packet;

    packet.id = buffer.getInt();
    packet.position.x = buffer.getFloat();
    packet.position.y = buffer.getFloat();
    packet.position.z = buffer.getFloat();

    return packet;
}

UpdateEntityClientPacket readUpdateEntityPacket(ByteBuffer buffer)
{
    UpdateEntityClientPacket packet;

    packet.id = buffer.getInt();
    packet.position.x = buffer.getFloat();
    packet.position.y = buffer.getFloat();
    packet.position.z = buffer.getFloat();
    packet.yaw = buffer.getFloat();
    packet.pitch = buffer.getFloat();

    return packet;
}

Client::Client(World& world, TextureManager& texture_manager, const char* ip):
    world(world)
{
    client_socket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(ip);
    serverAddress.sin_port = htons(15000);

    struct timeval tv;
    tv.tv_sec = 3;
    tv.tv_usec = 0;

    fd_set set;
    FD_ZERO(&set);
    FD_SET(client_socket, &set);

    // Set to blocking mode
    int opts = fcntl(client_socket, F_SETFL, O_NONBLOCK); // https://stackoverflow.com/questions/2597608/c-socket-connection-timeout

    printf("Connecting to %s...\n", ip);
    int res = connect(client_socket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
    if (errno != EINPROGRESS) {
        printf("Connection failed.\n");
        return;
    }

    res = select(client_socket+1, NULL, &set, NULL, &tv);

    if (res < 0 && errno != EINTR) {
        printf("Error connecting %d - %s\n", errno, strerror(errno));
        exit(0);
    } else if (res > 0) {
        printf("Successfully connected to %s\n", ip);
    } else {
        printf("Connection timeout.\n");
        exit(0);
    }

    // Set blocking mode back
    opts = opts & (~O_NONBLOCK);
    fcntl(client_socket, F_SETFL, opts);
}

Client::~Client() {
    if (client_thread.joinable())
        client_thread.detach(); /* Detach thread to avoid fatal error */
}

void Client::Start()
{
    client_thread = std::thread(&Client::clientThreadFunc, this);
}

void Client::packet_Identification(ByteBuffer buffer) {
    client_id = buffer.getInt();
    // printf("Client id: %d\n", client_id);
}

void Client::packet_AddEntity(ByteBuffer buffer) {
    auto [id, pos] = readAddEntityPacket(buffer);
    const std::lock_guard<std::mutex> lock(task_queue_mutex);
    task_queue.push_front([this, id, pos]() {
        Entity e{id};
        e.transform.position = pos;
        world.addEntity(e);
    } );
}

void Client::packet_RemoveEntity(ByteBuffer buffer) {
    int entityId = buffer.getInt();
    const std::lock_guard<std::mutex> lock(task_queue_mutex);
    task_queue.push_front([this, entityId]() {
        world.removeEntity(entityId);
    });
}

void Client::packet_UpdateEntity(ByteBuffer buffer) {
    auto [id, pos, yaw, pitch] = readUpdateEntityPacket(buffer);
    const std::lock_guard<std::mutex> lock(task_queue_mutex);
    task_queue.push_front([this, id, pos, yaw, pitch]() {
        world.setEntityTransform(id, pos, yaw, pitch);
    } );
}

void Client::packet_SendChunk(ByteBuffer buffer) {
    ChunkPacket* chunk_data = readChunkPacket(buffer);

    const std::lock_guard<std::mutex> lock(new_chunks_mutex);

    // Replace chunk if already in new chunk list to reduce charge on mainthread //
    int index_of_existing_chunk_pos = -1;
    for (uint i = 0 ; i < new_chunks.size() ; ++i) {
        if (new_chunks[i]->pos == chunk_data->pos) {
            index_of_existing_chunk_pos = i;
            break;
        }
    }
    if (index_of_existing_chunk_pos != -1) {
        delete new_chunks[index_of_existing_chunk_pos];
        new_chunks[index_of_existing_chunk_pos] = chunk_data;
    } else {
        new_chunks.push_front(chunk_data);
    }
}

void Client::packet_SendMonotypeChunk(ByteBuffer buffer) {
    ChunkPacket* chunk_data = readFullMonoChunkPacket(buffer);
    const std::lock_guard<std::mutex> lock(new_chunks_mutex);
    new_chunks.push_front(chunk_data);
}

void Client::clientThreadFunc()
{
    struct pollfd fds;
    fds.fd = client_socket;
    fds.events = POLLIN;

    uint8_t buffer[5000] = {0};

    while (true)
    {
        // usleep(10'000);

        int rv = poll(&fds, 1, 0); // poll (check if server sent anything)
        if (!(rv > 0 && (fds.revents & POLLIN))) continue;

        int recv_size = recv(client_socket, buffer, 1, 0);
        if (recv_size == -1) {
            std::cout << "recv failed: return -1" << std::endl;
            break;
        }

        uint8_t id = buffer[0];
        switch (id)
        {
            case 0x00: /* identification */
                recv_full(client_socket, buffer, 4);
                packet_Identification(ByteBuffer(buffer, 4, ByteBuffer::ByteOrder::BE));
                // printf("Client id: %d\n", client_id);
                break;
            case 0x01: /* add entity */
                recv_full(client_socket, buffer, 24);
                packet_AddEntity(ByteBuffer(buffer, 24, ByteBuffer::ByteOrder::BE));
                // printf("Server sent 'add entity' packet: %d %f %f %f.\n", id, pos.x, pos.y, pos.z);
                break;
            case 0x02: /* remove entity */
                recv_full(client_socket, buffer, 4);
                packet_RemoveEntity(ByteBuffer(buffer, 4, ByteBuffer::ByteOrder::BE));
                // printf("Server sent 'remove entity' packet: %d.\n", id);
                break;
            case 0x03: /* update entity */
                recv_full(client_socket, buffer, 24);
                packet_UpdateEntity(ByteBuffer(buffer, 24, ByteBuffer::ByteOrder::BE));
                // printf("Server sent 'move entity' packet: %d %f %f %f.\n", id, pos.x, pos.y, pos.z);
                break;
            case 0x04: /* Chunk */
                recv_full(client_socket, buffer, 4108);
                packet_SendChunk(ByteBuffer(buffer, 4108, ByteBuffer::ByteOrder::BE));
                // printf("Server sent 'chunk' packet: %d %d %d.\n", id, c.pos.x, c.pos.y, c.pos.z);
                break;
            case 0x05: /* full of same block chunk */
                recv_full(client_socket, buffer, 4*3+1);
                packet_SendMonotypeChunk(ByteBuffer(buffer, 4*3+1, ByteBuffer::ByteOrder::BE));
                break;
            default:
                break;
        }
    }
}

void Client::sendBreakBlockPacket(const glm::ivec3& world_pos)
{
    struct updateBlockServerPacket packet;

    packet.id = 0x01; // update block //
    packet.blockType = 0;

    packet.x = htobe32(*(uint32_t*)&world_pos.x);
    packet.y = htobe32(*(uint32_t*)&world_pos.y);
    packet.z = htobe32(*(uint32_t*)&world_pos.z);

    send(client_socket, &packet, sizeof(packet), 0);
}

void Client::sendBlockBulkEditPacket(const std::vector<glm::ivec3>& world_pos, BlockType blocktype)
{
    size_t size_in_bytes = sizeof(uint8_t) +
                            sizeof(uint32_t) +
                            world_pos.size() * (sizeof(uint8_t) + 3*sizeof(int32_t));

    auto buffer = std::make_unique<uint8_t[]>(size_in_bytes);
    uint8_t *head = &buffer[0];

    // id
    head[0] = 0x02;
    head += sizeof(uint8_t);

    // blockCount
    putIntBe(head, world_pos.size());
    head += sizeof(int32_t);

    for (size_t i = 0 ; i < world_pos.size() ; ++i)
    {
        head[0] = (uint8_t)blocktype;
        head += sizeof(uint8_t);

        putIntBe(head, world_pos[i].x);
        head += sizeof(int32_t);

        putIntBe(head, world_pos[i].y);
        head += sizeof(int32_t);

        putIntBe(head, world_pos[i].z);
        head += sizeof(int32_t);
    }

    send(client_socket, buffer.get(), size_in_bytes, 0);
}

void Client::sendPlaceBlockPacket(const glm::ivec3& world_pos, BlockType blocktype)
{
    struct updateBlockServerPacket packet;

    packet.id = 0x01; // update block //
    packet.blockType = (uint8_t)blocktype;
    packet.x = htobe32(*(uint32_t*)&world_pos.x);
    packet.y = htobe32(*(uint32_t*)&world_pos.y);
    packet.z = htobe32(*(uint32_t*)&world_pos.z);

    send(client_socket, &packet, sizeof(packet), 0);
}

void Client::sendUpdateEntityPacket(int entityId, const glm::vec3& pos, float yaw, float pitch)
{
    struct updateEntityServerPacket packet;

    packet.id = 0x00; // update entity //
    packet.entityId = htobe32(entityId);
    packet.x = htobe32(*(uint32_t*)&pos.x);
    packet.y = htobe32(*(uint32_t*)&pos.y);
    packet.z = htobe32(*(uint32_t*)&pos.z);
    packet.yaw = htobe32(*(uint32_t*)&yaw);
    packet.pitch = htobe32(*(uint32_t*)&pitch);

    send(client_socket, &packet, sizeof(packet), 0);
}
