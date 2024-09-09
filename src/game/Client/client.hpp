#pragma once

#include <unordered_map>
#include <thread>
#include <mutex>
#include <deque>

#include "enums.hpp"

#include "ClientPacket.hpp"
#include "ServerPacket.hpp"

class TextureManager;
class World;
class ByteBuffer;
struct Chunk;

class Client
{
    enum PacketId {
        IDENTIFICATION = 0x00,
        ADD_ENTITY = 0x01,
        REMOVE_ENTITY = 0x02,
        UPDATE_ENTITY = 0x03,
        CHUNK = 0x04,
        MONOTYPE_CHUNK = 0x05,
        CHAT_MESSAGE = 0x06,
        UPDATE_ENTITY_METADATA = 0x07
    };

public:
    Client(World& world, std::vector<std::string>& tchat, const char* ip);
    ~Client();

    // Client(const Client&) = delete;
    // Client& operator=(const Client&) = delete;
    // Client(Client&&) = delete;
    // Client& operator=(Client&&) = delete;

    void Start();
    void clientThreadFunc();

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
    World& world;
    std::vector<std::string>& tchat;
};
