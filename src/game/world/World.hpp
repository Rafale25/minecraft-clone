#pragma once

#include <vector>
#include <unordered_map>
#include <shared_mutex>
#include <atomic>

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include "Entity.hpp"
#include "enums.hpp"
#include "ServerPacket.hpp"

#include "BlockRaycastHit.hpp"


struct Chunk;

// class ChunkAllocator {
// public:
//     ChunkAllocator() = default;

// private:
//     // std::atomic<u_int64_t> id = 0;
//     static constexpr size_t MAX_CHUNKS = 10'000;
//     std::array<Chunk, MAX_CHUNKS> chunks;
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

    BlockRaycastHit blockRaycast(const glm::vec3& origin, const glm::vec3& direction, float maxDistance) const;

    BlockType getBlockf(const glm::vec3& posf) const;
    BlockType getBlock(const glm::ivec3& pos) const;

    Chunk* setChunk(const glm::ivec3& pos, const BlockType* blocks);
    void deleteChunk(const glm::ivec3 &pos);
    Chunk* getChunk(const glm::ivec3& pos) const;
    Chunk* getChunkUnsafe(const glm::ivec3 &pos) const; // Do not use mutex

    static World& instance() {
        static World instance;
        return instance;
    }

    int getChunkCount() const { return chunks.size(); }

public:
    // std::unordered_map<glm::ivec3, Chunk*, KeyHasher> chunks;
    std::unordered_map<glm::ivec3, Chunk*> chunks;
    mutable std::shared_mutex chunks_mutex;

    std::vector<Entity> entities;
    // std::unordered_map<int, Entity> entities; // TODO: switch to this data structure
};

// struct KeyHasher
// {
//     std::size_t operator()(const glm::ivec3& key) const
//     {
//         constexpr int SIZE = 25;
//         return key.x + key.y*SIZE + key.z*SIZE*SIZE;
//         // return ((key.x * 5209) ^ (key.y * 1811)) ^ (key.z * 7297);
//     }
// };
