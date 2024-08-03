#pragma once

#include <unordered_map>
#include <thread>
#include <mutex>
#include <deque>

#include "glm/gtx/hash.hpp"
#include "enums.hpp"

#include "ClientPacket.hpp"
#include "ServerPacket.hpp"

class TextureManager;
class World;
class ByteBuffer;
struct Chunk;

class Client
{
public:
    Client(World& world, std::vector<std::string>& tchat, const char* ip);
    ~Client();

    void Start();
    void clientThreadFunc();

    void sendBreakBlockPacket(const glm::ivec3& world_pos);
    void sendBlockBulkEditPacket(const std::vector<glm::ivec3>& world_pos, BlockType blocktype);
    void sendPlaceBlockPacket(const glm::ivec3& world_pos, BlockType blocktype);
    void sendUpdateEntityPacket(const glm::vec3& pos, float yaw, float pitch);
    void sendTextMessagePacket(const char *buffer);
    void sendClientMetadataPacket(int render_distance, std::string name);

    void readPacketIdentification(ByteBuffer buffer);
    void readPacketAddEntity(ByteBuffer buffer);
    void readPacketRemoveEntity(ByteBuffer buffer);
    void readPacketUpdateEntity(ByteBuffer buffer);
    void readPacketSendChunk(ByteBuffer buffer);
    void readPacketSendMonotypeChunk(ByteBuffer buffer);

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
