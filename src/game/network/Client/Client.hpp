#pragma once

#include "enums.hpp"
#include "ByteBuffer.hpp"
#include "TaskQueue.hpp"
#include "Network.hpp"
#include "ServerPacket.hpp"

#include <unordered_map>
#include <thread>
#include <mutex>
#include <deque>

class World;
struct Chunk;

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

// TODO: divide client into Connection and PacketManager
// Connection //
class Client
{

// static constexpr int32_t DEFAULT_PORT = 20000;

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
    void init(std::vector<std::string>& tchat, const char* ip, int32_t port);

    void Start();
    void Stop();

    void clientThreadFunc();

    void sendBreakBlockPacket(const glm::ivec3& world_pos);
    void sendBlockBulkEditPacket(const std::vector<std::tuple<glm::ivec3, BlockType>> blocks);
    void sendBlockBulkEditPacketMonotype(const std::vector<glm::ivec3>& world_pos, BlockType blocktype);
    void sendPlaceBlockPacket(const glm::ivec3& world_pos, BlockType blocktype);
    void sendUpdateEntityPacket(const glm::vec3& pos, float yaw, float pitch);
    void sendChatMessagePacket(const char *buffer);
    void sendClientMetadataPacket(int32_t render_distance, std::string name);

    static Client& instance() {
        static Client instance;
        return instance;
    }

private:
    void sendPacket(const void *buf, size_t size);

public:
    TaskQueue m_taskQueue;

    std::deque<Packet::Server::ChunkPacket*> m_newChunks;
    std::mutex m_newChunksMutex;

    int32_t m_clientId = -1;

private:
    NetworkConnection m_client;

    bool m_stopThread;
    std::thread m_clientThread;
    std::vector<std::string>* m_tchat;
};
