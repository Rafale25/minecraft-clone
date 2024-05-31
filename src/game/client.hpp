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


class Client
{
private:
    struct __attribute__ ((packed)) updateBlockPacket {
        uint8_t id;
        uint8_t blockType;
        int x, y, z;
    };

    struct __attribute__ ((packed)) updateEntityPacket {
        uint8_t id;
        int entityId;
        int x, y, z, yaw, pitch; // float encoded in int
    } packet;

public:
    Client() {};
    Client(World& world, TextureManager& texture_manager, const char* ip);

    void Start();
    void client_thread_func();

    void sendBreakBlockPacket(glm::ivec3 world_pos);
    void sendBlockBulkEditPacket(std::vector<glm::ivec3> &world_pos, BlockType blocktype);

    void sendPlaceBlockPacket(glm::ivec3 world_pos, BlockType blocktype);
    void sendUpdateEntityPacket(int entityId, glm::vec3 pos, float yaw, float pitch);

public:
    std::deque<std::function<void()>> task_queue;

    // NOTE: new chunk need to be assigned to the chunks list only when the VAO is computed
    std::deque<Chunk> new_chunks;
    std::mutex new_chunks_mutex;

    std::mutex task_queue_mutex;
    int client_id = -1;
    int client_socket;

private:
    std::thread client_thread;
    World *world;

    TextureManager* texture_manager;
};
