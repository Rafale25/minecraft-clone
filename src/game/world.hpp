#pragma once

#include <vector>
#include <unordered_map>
#include <mutex>

#include <glm/glm.hpp>
#include "glm/gtx/hash.hpp"

#include "entity.hpp"

struct Chunk;
class TextureManager;

class World
{
public:
    World();
    ~World();

    void add_entity(Entity e);
    void remove_entity(int id);
    void update_entity(int id, glm::vec3 pos, float yaw, float pitch);
    Entity* get_entity(int id);

public:
    std::unordered_map<glm::ivec3, Chunk> chunks;
    std::vector<Entity> entities;

    std::mutex chunks_mutex;
    std::mutex entities_mutex;

};
