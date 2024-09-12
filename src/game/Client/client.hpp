#pragma once

#include <unordered_map>
#include <thread>
#include <mutex>
#include <deque>

#include "enums.hpp"

#include "ClientPacket.hpp"
#include "ServerPacket.hpp"

#include "ByteBuffer.hpp"

#include "Singleton.hpp"

class TextureManager;
class World;
// class ByteBuffer;
struct Chunk;


void decodePacketEntity(ByteBuffer buffer);

// TODO: divide client into Connection and PacketManager
// Connection //
class Client: public Singleton<Client>
{
    friend Singleton<Client>;

private:
    Client() = default;
    ~Client();

    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;
    Client(Client&&) = delete;
    Client& operator=(Client&&) = delete;

    std::unordered_map<int, std::function<void(ByteBuffer)>> packets = {
        { 0x00, decodePacketEntity}
        // ...
    };


public:
    void init(std::vector<std::string>& tchat, const char* ip);
    void Start();
    void clientThreadFunc();

    void decode(PacketId id, ByteBuffer buffer);

    void sendBreakBlockPacket(const glm::ivec3& world_pos);
    void sendBlockBulkEditPacket(const std::vector<glm::ivec3>& world_pos, BlockType blocktype);
    void sendPlaceBlockPacket(const glm::ivec3& world_pos, BlockType blocktype);
    void sendUpdateEntityPacket(const glm::vec3& pos, float yaw, float pitch);
    void sendChatMessagePacket(const char *buffer);
    void sendClientMetadataPacket(int render_distance, std::string name);

    void readPacketIdentification(ByteBuffer buffer);
    void readPacketAddEntity(ByteBuffer buffer);
    void readPacketRemoveEntity(ByteBuffer buffer);
    void readPacketUpdateEntity(ByteBuffer buffer);
    void readPacketSendChunk(ByteBuffer buffer);
    void readPacketSendMonotypeChunk(ByteBuffer buffer);
    void readPacketEntityMetadata(ByteBuffer buffer);
    void readPacketChatMessage(const uint8_t* buffer);

    // static Client& instance() {
    //     static Client instance;
    //     return instance;
    // }

private:
    void sendPacket(const void *buf, size_t size);

public:
    std::deque<std::function<void()>> task_queue;
    std::mutex task_queue_mutex;

    std::deque<Packet::Server::ChunkPacket*> new_chunks;
    std::mutex new_chunks_mutex;

    int client_id = -1;
    int client_socket;

    std::thread client_thread;

private:
    std::vector<std::string>* _tchat;
};
