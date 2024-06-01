#pragma once

#include <vector>
#include <unordered_map>
#include <mutex>

#include <glm/glm.hpp>
#include "glm/gtx/hash.hpp"

#include "entity.hpp"
#include "enums.hpp"

struct Chunk;
class TextureManager;

class World
{
public:
    World();
    ~World();

    void add_entity(Entity e);
    void remove_entity(int id);
    void set_entity_transform(int id, glm::vec3 pos, float yaw, float pitch);
    Entity* get_entity(int id);
    void update_entities();

    // TODO: change maxsteps by a maxDistance
    std::tuple<BlockType, glm::ivec3, glm::vec3> BlockRaycast(glm::vec3 origin, glm::vec3 direction, int maxSteps);
    BlockType get_block(glm::ivec3 pos);

    // Chunk* create_chunk();
    // void delete_chunk();

    void set_chunk(Chunk* chunk);
    Chunk* get_chunk(glm::ivec3 pos);
    // std::unordered_map<glm::ivec3, Chunk*>::iterator chunks_iter();

public:
    std::unordered_map<glm::ivec3, Chunk*> chunks; //Note: do not access synchronously
    std::vector<Entity> entities;

    GLuint ssbo_texture_handles;
};
