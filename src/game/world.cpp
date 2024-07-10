#include "world.hpp"
#include "chunk.hpp"
#include "texture_manager.hpp"
// #include "entity.hpp"

#include <stdio.h>
#include <string.h>

World::World()
{
}

// World::~World()
// {
// }

Entity* World::getEntity(int id)
{
    for (size_t i = 0 ; i < entities.size() ; ++i)
    {
        if (entities[i].id == id)
            return &entities[i];
    }
    return nullptr;
}

void World::updateEntities()
{
    const float smoothness = 0.075f;
    for (auto& entity : entities)
    {
        entity.smooth_transform.position = glm::mix(entity.smooth_transform.position, entity.transform.position, smoothness);
        entity.smooth_transform.rotation = glm::mix(entity.smooth_transform.rotation, entity.transform.rotation, smoothness);
    }
}

void World::addEntity(Entity e)
{
    entities.push_back(e);
}

void World::removeEntity(int id)
{
    for (size_t i = 0 ; i < entities.size() ; ++i)
    {
        if (entities[i].id == id) {
            entities.erase(entities.begin() + i);
            // delete entity ?
        }
    }
}

void World::setEntityTransform(int id, const glm::vec3& pos, float yaw, float pitch)
{
    Entity* e = getEntity(id);
    if (e == nullptr) return;
    e->transform.position = pos;
    e->transform.rotation = glm::quat(glm::vec3(-pitch, -yaw, 0.0f));
}

BlockType World::getBlock(glm::ivec3 pos) const
{
    // TODO: try using static variables since this function is hot
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

BlockRaycastHit World::BlockRaycast(const glm::vec3& origin, const glm::vec3& direction, int maxSteps) const
{
    glm::vec3 rayPos = origin;
    glm::vec3 mapPos = glm::ivec3(glm::floor(rayPos));

    glm::vec3 deltaDist = abs(glm::vec3(glm::length(direction)) / direction);
	glm::vec3 rayStep = sign(direction);
	glm::vec3 sideDist = (sign(direction) * (mapPos - rayPos) + (sign(direction) * 0.5f) + 0.5f) * deltaDist;

	glm::vec3 mask;
    glm::vec3 normal;

	for (int i = 0; i < maxSteps; i++) {
        auto block = getBlock(mapPos);
        normal = -mask * rayStep;

        if (block != BlockType::Air) {
            return {block, mapPos, normal};
        };

		mask = glm::step(sideDist, glm::vec3(sideDist.y, sideDist.z, sideDist.x)) * glm::step(sideDist, glm::vec3(sideDist.z, sideDist.x, sideDist.y));
		sideDist += mask * deltaDist;
		mapPos += mask * rayStep;
	}

    return {BlockType::Air, mapPos, normal};
}

Chunk* World::setChunk(ChunkData* chunk_data)
{
    Chunk* chunk = nullptr;

    if (chunks.find(chunk_data->pos) == chunks.end()) { // if not found
        chunk = new Chunk();
        chunk->pos = chunk_data->pos;
        chunks[chunk_data->pos] = chunk;
    } else {
        chunk = chunks[chunk_data->pos];
    }

    memcpy(chunk->blocks, chunk_data->blocks, 4096 * sizeof(uint8_t));

    return chunk;
}

Chunk* World::getChunk(glm::ivec3 pos) const
{
    if (chunks.find(pos) != chunks.end())
        return chunks[pos];
    return nullptr;
}
