#include <iostream>
#include <functional>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <fcntl.h>
#include <sys/select.h>
#include <cstring>
#include <endian.h>

#include <string>

#include "client.hpp"
#include "chunk.hpp"
#include "world.hpp"
#include "entity.hpp"
#include "utils/print_buffer.hpp"

#include "utils/recv_full.hpp"
#include "utils/byte_manipulation.hpp"

#include "ByteBuffer.hpp"

Packet::Server::ChunkPacket* readChunkPacket(ByteBuffer& buffer)
{
    auto* chunk_data = new Packet::Server::ChunkPacket;

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

Packet::Server::ChunkPacket* readFullMonoChunkPacket(ByteBuffer buffer)
{
    int x = buffer.getInt();
    int y = buffer.getInt();
    int z = buffer.getInt();
    uint8_t blockType = buffer.get();

    auto *chunk_data = new Packet::Server::ChunkPacket;
    chunk_data->pos = glm::ivec3(x, y, z) / 16;
    memset(chunk_data->blocks, blockType, 16*16*16);

    return chunk_data;
}

Packet::Server::AddEntity readAddEntityPacket(ByteBuffer buffer)
{
    Packet::Server::AddEntity packet;

    packet.id = buffer.getInt();
    packet.position.x = buffer.getFloat();
    packet.position.y = buffer.getFloat();
    packet.position.z = buffer.getFloat();
    packet.yaw = buffer.getFloat();
    packet.pitch = buffer.getFloat();
    buffer.getN((uint8_t*)packet.name, 64);

    return packet;
}

Packet::Server::UpdateEntity readUpdateEntityPacket(ByteBuffer buffer)
{
    Packet::Server::UpdateEntity packet;

    packet.entity_id = buffer.getInt();
    packet.position.x = buffer.getFloat();
    packet.position.y = buffer.getFloat();
    packet.position.z = buffer.getFloat();
    packet.yaw = buffer.getFloat();
    packet.pitch = buffer.getFloat();

    return packet;
}

Client::Client(std::vector<std::string>& tchat, const char* ip):
    tchat(tchat)
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

    sendClientMetadataPacket(16, "Rafale25");
}

Client::~Client() {
    if (client_thread.joinable())
        client_thread.detach(); /* Detach thread to avoid fatal error */
}

void Client::Start()
{
    client_thread = std::thread(&Client::clientThreadFunc, this);
}

void Client::clientThreadFunc()
{
    struct pollfd fds;
    fds.fd = client_socket;
    fds.events = POLLIN;

    uint8_t buffer[5000] = {0};

    while (true)
    {
        int rv = poll(&fds, 1, 0); // poll (check if server sent anything)
        if (!(rv > 0 && (fds.revents & POLLIN))) continue;

        int recv_size = recv(client_socket, buffer, 1, 0);
        if (recv_size == -1) {
            std::cout << "recv failed: return -1" << std::endl;
            break;
        }

        using namespace Packet;

        uint8_t id = buffer[0];
        switch (id)
        {
            case IDENTIFICATION:
                recv_full(client_socket, buffer, sizeof(Server::Identification));
                readPacketIdentification(ByteBuffer(buffer, sizeof(Server::Identification), ByteBuffer::ByteOrder::BE));
                break;
            case ADD_ENTITY:
                recv_full(client_socket, buffer, sizeof(Server::AddEntity));
                readPacketAddEntity(ByteBuffer(buffer, sizeof(Server::AddEntity), ByteBuffer::ByteOrder::BE));
                break;
            case REMOVE_ENTITY:
                recv_full(client_socket, buffer, sizeof(Server::RemoveEntity));
                readPacketRemoveEntity(ByteBuffer(buffer, sizeof(Server::RemoveEntity), ByteBuffer::ByteOrder::BE));
                break;
            case UPDATE_ENTITY:
                recv_full(client_socket, buffer, sizeof(Server::UpdateEntity));
                readPacketUpdateEntity(ByteBuffer(buffer, sizeof(Server::UpdateEntity), ByteBuffer::ByteOrder::BE));
                break;
            case CHUNK:
                recv_full(client_socket, buffer, sizeof(Server::ChunkPacket));
                readPacketSendChunk(ByteBuffer(buffer, sizeof(Server::ChunkPacket), ByteBuffer::ByteOrder::BE));
                break;
            case MONOTYPE_CHUNK:
                recv_full(client_socket, buffer, sizeof(Server::MonoChunkPacket));
                readPacketSendMonotypeChunk(ByteBuffer(buffer, sizeof(Server::MonoChunkPacket), ByteBuffer::ByteOrder::BE));
                break;
            case CHAT_MESSAGE:
                recv_full(client_socket, buffer, sizeof(Server::ChatMessage));
                readPacketChatMessage(buffer);
                break;
            case UPDATE_ENTITY_METADATA:
                recv_full(client_socket, buffer, sizeof(Server::UpdateEntityMetadata));
                // TODO: Do stuff with it
                break;
            default:
                break;
        }
    }
}

// RECEIVE //

void Client::readPacketIdentification(ByteBuffer buffer) {
    client_id = buffer.getInt();
}

void Client::readPacketAddEntity(ByteBuffer buffer) {
    auto [id, pos, yaw, pitch, name] = readAddEntityPacket(buffer);
    const std::lock_guard<std::mutex> lock(task_queue_mutex);
    task_queue.push_front([&]() {
        Entity e{id};
        e.transform.position = pos;
        // e.transform.rotation.y = yaw;
        // e.transform.rotation.x = pitch;
        e.name = std::string(name);
        World::instance().addEntity(e);
    } );
}

void Client::readPacketRemoveEntity(ByteBuffer buffer) {
    int entity_id = buffer.getInt();
    const std::lock_guard<std::mutex> lock(task_queue_mutex);
    task_queue.push_front([&]() {
        World::instance().removeEntity(entity_id);
    });
}

void Client::readPacketUpdateEntity(ByteBuffer buffer) {
    auto [id, pos, yaw, pitch] = readUpdateEntityPacket(buffer);
    const std::lock_guard<std::mutex> lock(task_queue_mutex);
    task_queue.push_front([&]() {
        World::instance().setEntityTransform(id, pos, yaw, pitch);
    } );
}

void Client::readPacketSendChunk(ByteBuffer buffer) {
    auto* chunk_data = readChunkPacket(buffer);

    const std::lock_guard<std::mutex> lock(new_chunks_mutex);

    // Replace chunk if already in new chunk list to reduce charge on mainthread //
    auto it = std::find_if(new_chunks.begin(), new_chunks.end(), [&](const auto& chunk){ return chunk->pos == chunk_data->pos; });
    if (it != new_chunks.end()) {
        delete *it;
        *it = chunk_data;
    } else {
        new_chunks.push_front(chunk_data);
    }
}

void Client::readPacketSendMonotypeChunk(ByteBuffer buffer) {
    auto* chunk_data = readFullMonoChunkPacket(buffer);
    const std::lock_guard<std::mutex> lock(new_chunks_mutex);
    new_chunks.push_front(chunk_data);
}

void Client::readPacketEntityMetadata(ByteBuffer buffer) {
    auto [id, pos, yaw, pitch] = readUpdateEntityPacket(buffer);
    const std::lock_guard<std::mutex> lock(task_queue_mutex);
    task_queue.push_front([&]() {
        World::instance().setEntityTransform(id, pos, yaw, pitch);
    } );
}

void Client::readPacketChatMessage(const uint8_t* buffer) {
    std::string str = std::string((char*)buffer, strlen((char*)buffer));

    /* Removes &[] from message */
    size_t index = 0;
    while (true) {
        /* Locate the substring to replace. */
        index = str.find("&", index);
        if (index == std::string::npos) break;

        /* Make the replacement. */
        str.replace(index, 2, "");

        /* Advance index forward so the next iteration doesn't pick it up as well. */
        index += 2;
    }

    tchat.push_back(str);
}

// SEND //

void Client::sendBreakBlockPacket(const glm::ivec3& world_pos)
{
    Packet::Client::UpdateBlock packet = {};

    packet.id = Packet::Client::PACKET_EDIT_BLOCK; // update block //
    packet.blockType = (uint8_t)BlockType::Air;

    packet.x = htobe32(*(uint32_t*)&world_pos.x);
    packet.y = htobe32(*(uint32_t*)&world_pos.y);
    packet.z = htobe32(*(uint32_t*)&world_pos.z);

    sendPacket(&packet, sizeof(packet));
}

void Client::sendBlockBulkEditPacket(const std::vector<glm::ivec3>& world_pos, BlockType blocktype)
{
    size_t size_in_bytes = sizeof(uint8_t) +
                            sizeof(uint32_t) +
                            world_pos.size() * (sizeof(uint8_t) + 3*sizeof(int32_t));

    auto buffer = std::make_unique<uint8_t[]>(size_in_bytes);
    uint8_t *head = &buffer[0];

    // id
    head[0] = Packet::Client::PACKET_EDIT_BLOCK_BULK;
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

    sendPacket(buffer.get(), size_in_bytes);
}

void Client::sendPlaceBlockPacket(const glm::ivec3& world_pos, BlockType blocktype)
{
    Packet::Client::UpdateBlock packet = {};

    packet.id = Packet::Client::PACKET_EDIT_BLOCK; // update block //
    packet.blockType = (uint8_t)blocktype;
    packet.x = htobe32(*(uint32_t*)&world_pos.x);
    packet.y = htobe32(*(uint32_t*)&world_pos.y);
    packet.z = htobe32(*(uint32_t*)&world_pos.z);

    sendPacket(&packet, sizeof(packet));
}

void Client::sendUpdateEntityPacket(const glm::vec3& pos, float yaw, float pitch)
{
    Packet::Client::UpdateEntity packet = {};

    packet.id = Packet::Client::PACKET_UPDATE_ENTITY; // update entity //
    packet.x = htobe32(*(uint32_t*)&pos.x);
    packet.y = htobe32(*(uint32_t*)&pos.y);
    packet.z = htobe32(*(uint32_t*)&pos.z);
    packet.yaw = htobe32(*(uint32_t*)&yaw);
    packet.pitch = htobe32(*(uint32_t*)&pitch);

    sendPacket(&packet, sizeof(packet));
}

void Client::sendChatMessagePacket(const char* buffer)
{
    Packet::Client::ChatMessage packet = {};

    size_t size = strlen(buffer) * sizeof(char);
    assert(size <= sizeof(packet.buffer));

    packet.id = Packet::Client::PACKET_TEXT_MESSAGE;
    memcpy(packet.buffer, buffer, size);

    sendPacket(&packet, sizeof(packet));
}

void Client::sendClientMetadataPacket(int render_distance, std::string name)
{
    Packet::Client::ClientMetadata packet = {};

    packet.id = Packet::Client::PACKET_CLIENT_METADATA;
    packet.render_distance = render_distance;
    memcpy(packet.name, name.c_str(), name.length());

    sendPacket(&packet, sizeof(packet));
}

void Client::sendPacket(const void *buf, size_t size)
{
    send(client_socket, buf, size, 0);
}
