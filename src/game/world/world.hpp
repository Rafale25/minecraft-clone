#pragma once

#include <vector>
#include <unordered_map>
#include <shared_mutex>

#include <glm/glm.hpp>
#include "glm/gtx/hash.hpp"

#include "entity.hpp"
#include "enums.hpp"
#include "ServerPacket.hpp"

struct Chunk;
class TextureManager;

#include "BlockRaycastHit.hpp"

// struct KeyHasher
// {
//     std::size_t operator()(const glm::ivec3& key) const
//     {
//         constexpr int SIZE = 25;
//         return key.x + key.y*SIZE + key.z*SIZE*SIZE;
//         // return ((key.x * 5209) ^ (key.y * 1811)) ^ (key.z * 7297);
//     }
// };


class World
{
private:
    World();
    ~World() = default;

    World(const World&) = delete;
    World& operator=(const World&) = delete;
    World(World&&) = delete;
    World& operator=(World&&) = delete;

public:
    void addEntity(Entity e);
    void removeEntity(int id);
    void setEntityTransform(int id, const glm::vec3& pos, float yaw, float pitch);
    void setEntityName(int id, std::string name);
    Entity* getEntity(int id);
    void updateEntities();

    // TODO: change maxsteps by a maxDistance
    BlockRaycastHit BlockRaycast(const glm::vec3& origin, const glm::vec3& direction, int maxSteps) const;
    BlockType getBlock(const glm::ivec3& pos) const;

    Chunk* setChunk(Packet::Server::ChunkPacket* chunk_data);
    Chunk* getChunk(const glm::ivec3& pos) const;

    static World& instance() {
        static World instance;
        return instance;
    }

public:
    std::unordered_map<glm::ivec3, Chunk*> chunks;
    // std::unordered_map<glm::ivec3, Chunk*, KeyHasher> chunks;
    mutable std::shared_mutex chunks_mutex;

    std::vector<Entity> entities;
    // std::unordered_map<int, Entity> entities; // TODO: switch to this data structure

};
