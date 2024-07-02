#pragma once

#include <vector>
#include <unordered_map>
#include <mutex>

#include <glm/glm.hpp>
#include "glm/gtx/hash.hpp"

#include "entity.hpp"
#include "enums.hpp"
#include "client.hpp" // TODO: put chunk_data in its own header file

struct Chunk;
class TextureManager;

class World
{
public:
    World();
    ~World();

    void addEntity(Entity e);
    void removeEntity(int id);
    void setEntityTransform(int id, glm::vec3 pos, float yaw, float pitch);
    Entity* getEntity(int id);
    void updateEntities();

    // TODO: change maxsteps by a maxDistance
    std::tuple<BlockType, glm::ivec3, glm::vec3> BlockRaycast(glm::vec3 origin, glm::vec3 direction, int maxSteps);
    BlockType getBlock(glm::ivec3 pos);

    // Chunk* create_chunk();
    // void delete_chunk();

    void setChunk(ChunkData* chunk_data, TextureManager& texture_manager);
    Chunk* getChunk(glm::ivec3 pos);
    // std::unordered_map<glm::ivec3, Chunk*>::iterator chunks_iter();

public:
    std::unordered_map<glm::ivec3, Chunk*> chunks; //Note: do not access synchronously
    std::vector<Entity> entities;

    GLuint ssbo_texture_handles;
};
