#pragma once

#include <vector>
#include <unordered_map>
#include <mutex>

#include <glm/glm.hpp>
#include "glm/gtx/hash.hpp"

#include "entity.hpp"
#include "enums.hpp"
#include "client.hpp"

struct Chunk;
class TextureManager;

struct BlockRaycastHit {
    BlockType blocktype;
    glm::ivec3 pos;
    glm::vec3 normal;
};

class World
{
public:
    World();
    ~World() = default;

    void addEntity(Entity e);
    void removeEntity(int id);
    void setEntityTransform(int id, const glm::vec3& pos, float yaw, float pitch);
    Entity* getEntity(int id);
    void updateEntities();

    // TODO: change maxsteps by a maxDistance
    BlockRaycastHit BlockRaycast(const glm::vec3& origin, const glm::vec3& direction, int maxSteps) const;
    BlockType getBlock(const glm::ivec3& pos) const;

    //  getBlocks(const glm::ivec3& pos) const;

    // Chunk* create_chunk();
    // void delete_chunk();

    // void setChunk(Chunk* chunk_data);
    Chunk* setChunk(ChunkPacket* chunk_data);
    Chunk* getChunk(const glm::ivec3& pos) const;

public:
    mutable std::unordered_map<glm::ivec3, Chunk*> chunks;
    std::mutex chunks_mutex;

    std::vector<Entity> entities;
    // std::unordered_map<int, Entity> entities; // TODO: switch to this data structure
};
