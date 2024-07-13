#pragma once

#include <unordered_map>
#include <thread>
#include <mutex>
#include <deque>

#include "glm/gtx/hash.hpp"
#include "enums.hpp"

class TextureManager;
class World;
struct Chunk;

 // TODO: put these structs in their own header file

struct ChunkPacket { // sendChunkClientPacket
    glm::ivec3 pos;
    BlockType blocks[4096];
};

struct UpdateEntityClientPacket {
    int id;
    glm::vec3 position;
    float yaw;
    float pitch;
};

struct AddEntityClientPacket {
    int id;
    glm::vec3 position;
};

struct __attribute__ ((packed)) updateBlockServerPacket {
    uint8_t id;
    uint8_t blockType;
    int x, y, z;
};

struct __attribute__ ((packed)) updateEntityServerPacket {
    uint8_t id;
    int entityId;
    int x, y, z, yaw, pitch; // float encoded in int
};

class Client
{
public:
    Client(World& world, TextureManager& texture_manager, const char* ip);
    ~Client();

    void Start();
    void clientThreadFunc();

    void sendBreakBlockPacket(const glm::ivec3& world_pos);
    void sendBlockBulkEditPacket(const std::vector<glm::ivec3>& world_pos, BlockType blocktype);

    void sendPlaceBlockPacket(const glm::ivec3& world_pos, BlockType blocktype);
    void sendUpdateEntityPacket(int entityId, const glm::vec3& pos, float yaw, float pitch);

    void packet_Identification(uint8_t* buffer);
    void packet_AddEntity(uint8_t* buffer);
    void packet_RemoveEntity(uint8_t* buffer);
    void packet_UpdateEntity(uint8_t* buffer);
    void packet_SendChunk(uint8_t* buffer);
    void packet_SendMonotypeChunk(uint8_t* buffer);

public:
    std::deque<std::function<void()>> task_queue;
    std::mutex task_queue_mutex;

    // NOTE: new chunk need to be assigned to the chunks list only when the VAO is computed
    std::deque<ChunkPacket*> new_chunks;
    std::mutex new_chunks_mutex;

    int client_id = -1;
    int client_socket;

    std::thread client_thread;
private:
    World& world;
};
