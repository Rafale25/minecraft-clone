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

    // Chunk* create_chunk();
    // void delete_chunk();

    BlockType get_block(glm::ivec3 pos);

public:
    std::unordered_map<glm::ivec3, Chunk> chunks; //Note: do not access synchronously
    std::vector<Entity> entities;

    // std::mutex entities_mutex;
};
