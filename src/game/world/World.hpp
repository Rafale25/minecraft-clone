#pragma once

#include "Entity.hpp"
#include "enums.hpp"
#include "constants.hpp"
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_int3.hpp>
#include <glm/gtx/hash.hpp>
#include <vector>
#include <unordered_map>
#include <shared_mutex>

struct Chunk;
struct BlockRaycastHit;

// #include <atomic>
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
    void removeEntity(int32_t id);
    void setEntityTransform(int32_t id, const glm::vec3& pos, float yaw, float pitch);
    void setEntityName(int32_t id, std::string name);
    Entity* getEntity(int32_t id);
    void updateEntities();

    BlockRaycastHit blockRaycast(const glm::vec3& origin, const glm::vec3& direction, float maxDistance) const;

    BlockType getBlock(const glm::vec3& posf) const;
    BlockType getBlock(const glm::ivec3& pos) const;

    Chunk* setChunk(const glm::ivec3& pos, const BlockType* blocks);
    void deleteChunk(const glm::ivec3 &pos);
    Chunk* getChunk(const glm::ivec3& pos) const;
    Chunk* getChunkUnsafe(const glm::ivec3 &pos) const; // Do not use mutex

    static glm::ivec3 worldToChunkCoord(const glm::vec3& p) {
        return glm::floor(p / CHUNK_SIZEF);
    }

    static World& instance() {
        static World instance;
        return instance;
    }

    int32_t getChunkCount() const { return m_chunks.size(); }

public:
    std::unordered_map<glm::ivec3, Chunk*> m_chunks;
    mutable std::shared_mutex m_chunksMutex;

    std::vector<Entity> m_entities;
    // std::unordered_map<int32_t, Entity> entities; // TODO: switch to this data structure
};
