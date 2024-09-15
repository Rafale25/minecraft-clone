#pragma once

#include <unordered_map>
#include <thread>
#include <mutex>
#include <deque>

#include "enums.hpp"

#include "ClientPacket.hpp"
#include "ServerPacket.hpp"

#include "ByteBuffer.hpp"

class World;
struct Chunk;

// TODO: divide client into Connection and PacketManager
// Connection //
class Client
{

private:
    Client() = default;
    ~Client() = default;

    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;
    Client(Client&&) = delete;
    Client& operator=(Client&&) = delete;

    static void decodePacketIdentification(ByteBuffer buffer);
    static void decodePacketAddEntity(ByteBuffer buffer);
    static void decodePacketRemoveEntity(ByteBuffer buffer);
    static void decodePacketUpdateEntity(ByteBuffer buffer);
    static void decodePacketChunk(ByteBuffer buffer);
    static void decodePacketMonotypeChunk(ByteBuffer buffer);
    static void decodePacketEntityMetadata(ByteBuffer buffer);
    static void decodePacketChatMessage(ByteBuffer buffer);

    void decode(PacketId id, ByteBuffer buffer);

    struct PacketInfo {
        std::function<void(ByteBuffer)> decode;
        size_t size;
    };

    const std::unordered_map<PacketId, PacketInfo> packets = {
        { PacketId::IDENTIFICATION,             { decodePacketIdentification,   sizeof(Packet::Server::Identification)        } },
        { PacketId::ADD_ENTITY,                 { decodePacketAddEntity,        sizeof(Packet::Server::AddEntity)             } },
        { PacketId::REMOVE_ENTITY,              { decodePacketRemoveEntity,     sizeof(Packet::Server::RemoveEntity)          } },
        { PacketId::UPDATE_ENTITY,              { decodePacketUpdateEntity,     sizeof(Packet::Server::UpdateEntity)          } },
        { PacketId::CHUNK,                      { decodePacketChunk,            sizeof(Packet::Server::ChunkPacket)           } },
        { PacketId::MONOTYPE_CHUNK,             { decodePacketMonotypeChunk,    sizeof(Packet::Server::MonoChunkPacket)       } },
        { PacketId::CHAT_MESSAGE,               { decodePacketChatMessage,      sizeof(Packet::Server::ChatMessage)           } },
        { PacketId::UPDATE_ENTITY_METADATA,     { decodePacketEntityMetadata,   sizeof(Packet::Server::UpdateEntityMetadata)  } },
    };

public:
    void init(std::vector<std::string>& tchat, const char* ip);

    void Start();
    void Stop();

    void clientThreadFunc();

    void sendBreakBlockPacket(const glm::ivec3& world_pos);
    void sendBlockBulkEditPacket(const std::vector<glm::ivec3>& world_pos, BlockType blocktype);
    void sendPlaceBlockPacket(const glm::ivec3& world_pos, BlockType blocktype);
    void sendUpdateEntityPacket(const glm::vec3& pos, float yaw, float pitch);
    void sendChatMessagePacket(const char *buffer);
    void sendClientMetadataPacket(int render_distance, std::string name);

    static Client& instance() {
        static Client instance;
        return instance;
    }

private:
    void sendPacket(const void *buf, size_t size);

public:
    std::deque<std::function<void()>> task_queue;
    std::mutex task_queue_mutex;

    std::deque<Packet::Server::ChunkPacket*> new_chunks;
    std::mutex new_chunks_mutex;

    int client_id = -1;

private:
    bool _stop_thread;
    int client_socket;
    std::thread client_thread;
    std::vector<std::string>* _tchat;
};
