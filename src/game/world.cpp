#include "world.hpp"
#include "chunk.hpp"
#include "texture_manager.hpp"
// #include "entity.hpp"

World::World()
{
}

World::~World()
{
}

Entity* World::get_entity(int id)
{
    for (size_t i = 0 ; i < entities.size() ; ++i)
    {
        if (entities[i].id == id)
            return &entities[i];
    }
    return nullptr;
}

void World::add_entity(Entity e)
{
    entities.push_back(e);
}

void World::remove_entity(int id)
{
    for (size_t i = 0 ; i < entities.size() ; ++i)
    {
        if (entities[i].id == id) {
            entities.erase(entities.begin() + i);
            // delete entity ?
        }
    }
}

void World::update_entity(int id, glm::vec3 pos, float yaw, float pitch)
{
    Entity* e = get_entity(id);
    if (e == nullptr) return;
    e->transform.position = pos;
    e->transform.rotation = glm::quat(glm::vec3(-pitch, -yaw, 0.0f));
}

BlockType World::get_block(glm::ivec3 pos)
{
    glm::ivec3 chunk_pos = pos / 16;
    glm::ivec3 local_pos = {pos.x % 16, pos.y % 16, pos.z % 16};

    if (chunks.count(chunk_pos) == 0) return BlockType::Air; // chunk doesn't exist //

    int index = Chunk::XYZtoIndex(local_pos.x, local_pos.y, local_pos.z);
    return chunks[chunk_pos].blocks[index];
}
