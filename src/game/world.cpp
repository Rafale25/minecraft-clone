#include "world.hpp"
#include "chunk.hpp"
#include "texture_manager.hpp"
// #include "entity.hpp"

#include <stdio.h>

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

void World::update_entities()
{
    const float smoothness = 0.075f;
    for (auto& entity : entities)
    {
        entity.smooth_transform.position = glm::mix(entity.smooth_transform.position, entity.transform.position, smoothness);
        entity.smooth_transform.rotation = glm::mix(entity.smooth_transform.rotation, entity.transform.rotation, smoothness);
    }
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

void World::set_entity_transform(int id, glm::vec3 pos, float yaw, float pitch)
{
    Entity* e = get_entity(id);
    if (e == nullptr) return;
    e->transform.position = pos;
    e->transform.rotation = glm::quat(glm::vec3(-pitch, -yaw, 0.0f));
}

BlockType World::get_block(glm::ivec3 pos)
{
    glm::ivec3 chunk_pos = glm::floor(glm::vec3(pos) / 16.0f);
    glm::ivec3 local_pos = {pos.x % 16, pos.y % 16, pos.z % 16};
    if (local_pos.x < 0) local_pos.x += 16;
    if (local_pos.y < 0) local_pos.y += 16;
    if (local_pos.z < 0) local_pos.z += 16;

    // printf("chunck pos: %d %d %d\n", chunk_pos.x, chunk_pos.y, chunk_pos.z);
    // printf("local_pos: %d %d %d\n", local_pos.x, local_pos.y, local_pos.z);
    if (chunks.count(chunk_pos) == 0) return BlockType::Air; // chunk doesn't exist //

    int index = Chunk::XYZtoIndex(local_pos.x, local_pos.y, local_pos.z);
    return chunks[chunk_pos]->blocks[index];
}

std::tuple<BlockType, glm::ivec3, glm::vec3> World::BlockRaycast(glm::vec3 origin, glm::vec3 direction, int maxSteps)
{
    glm::vec3 rayPos = origin;
    glm::vec3 mapPos = glm::ivec3(glm::floor(rayPos));

    glm::vec3 deltaDist = abs(glm::vec3(glm::length(direction)) / direction);
	glm::vec3 rayStep = sign(direction);
	glm::vec3 sideDist = (sign(direction) * (mapPos - rayPos) + (sign(direction) * 0.5f) + 0.5f) * deltaDist;

	glm::vec3 mask;
    glm::vec3 normal;

	for (int i = 0; i < maxSteps; i++) {
        auto block = get_block(mapPos);
        normal = -mask * rayStep;

        if (block != BlockType::Air) {
            return std::tuple<BlockType, glm::ivec3, glm::vec3>({block, mapPos, normal});
        };

		mask = glm::step(sideDist, glm::vec3(sideDist.y, sideDist.z, sideDist.x)) * glm::step(sideDist, glm::vec3(sideDist.z, sideDist.x, sideDist.y));
		sideDist += mask * deltaDist;
		mapPos += mask * rayStep;
	}

    return std::tuple<BlockType, glm::ivec3, glm::vec3>({BlockType::Air, mapPos, normal});
}

void World::set_chunk(Chunk* chunk)
{
    if (chunks.find(chunk->pos) != chunks.end()) {
        if (chunks[chunk->pos]->mesh.is_initialized) {
            chunks[chunk->pos]->mesh.delete_all();
        }
        free(chunks[chunk->pos]);
        // TODO: call delete_chunk() instead of free directly
    }

    chunks[chunk->pos] = chunk;
}

Chunk* World::get_chunk(glm::ivec3 pos)
{
    //NOTE: Maybe can return directly chunks[pos] because unordered_map will call default constructor for Chunk* which is probably 0
    if (chunks.find(pos) != chunks.end()) {
        return chunks[pos];
    }
    return nullptr;
}

// std::unordered_map<glm::ivec3, Chunk *>::iterator World::chunks_iter()
// {
//     return chunks.begin();
// }
