#include "World.hpp"
#include "Chunk.hpp"

#include <stdio.h>
#include <string.h>

#include "VAO.hpp"

World::World()
{
    chunk_vao = createVAO(0, "i");
}

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

void World::setEntityName(int id, std::string name)
{
    Entity* e = getEntity(id);
    if (e == nullptr) return;
    e->name = name;
}

BlockType World::getBlock(const glm::ivec3& pos) const
{
    // TODO: try using static variables since this function is hot
    glm::ivec3 chunk_pos = glm::floor(glm::vec3(pos) / 16.0f);
    glm::ivec3 local_pos = {pos.x % 16, pos.y % 16, pos.z % 16};
    if (local_pos.x < 0) local_pos.x += 16;
    if (local_pos.y < 0) local_pos.y += 16;
    if (local_pos.z < 0) local_pos.z += 16;

    // printf("chunck pos: %d %d %d\n", chunk_pos.x, chunk_pos.y, chunk_pos.z);
    // printf("local_pos: %d %d %d\n", local_pos.x, local_pos.y, local_pos.z);
    auto it = chunks.find(chunk_pos);
    if (it == chunks.end()) return BlockType::Air; // chunk doesn't exist //

    int index = Chunk::XYZtoIndex(local_pos.x, local_pos.y, local_pos.z);
    return it->second->blocks[index];
}

BlockRaycastHit World::blockRaycast(const glm::vec3& origin, const glm::vec3& direction, float maxDistance) const
{
    const glm::vec3 rayPos = origin;
    const glm::vec3 deltaDist = abs(glm::vec3(glm::length(direction)) / direction);
	const glm::vec3 rayStep = sign(direction);

    glm::vec3 mapPos = glm::ivec3(glm::floor(rayPos));

    glm::vec3 sideDist = (sign(direction) * (mapPos - rayPos) + (sign(direction) * 0.5f) + 0.5f) * deltaDist;
	glm::vec3 mask;
    glm::vec3 normal;

    #define MAX_ITERATION 500
	for (int i = 0; i < MAX_ITERATION; i++) {
        auto block = getBlock(mapPos);
        normal = -mask * rayStep;

        if (block != BlockType::Air) {
            return {block, mapPos, normal};
        };

		mask = glm::step(sideDist, glm::vec3(sideDist.y, sideDist.z, sideDist.x)) * glm::step(sideDist, glm::vec3(sideDist.z, sideDist.x, sideDist.y));
		sideDist += mask * deltaDist;
		mapPos += mask * rayStep;

        if (glm::distance(rayPos, mapPos) > maxDistance) {
            break;
        }
	}

    return {BlockType::Air, mapPos, normal};
}

// #include "clock.hpp"
uint hashBlocks(const uint8_t* values) {
    uint h = 1;

    for (int i = 0 ; i < 4096 ; ++i) {
        h *= (1779033703 + 2*(uint)values[i]);
    }

    return h;
}

Chunk* World::setChunk(Packet::Server::ChunkPacket* chunk_data)
{
    Chunk* chunk = nullptr;

    chunks_mutex.lock_shared();
    auto it = chunks.find(chunk_data->pos);
    chunks_mutex.unlock_shared();

    if (it == chunks.end()) { // if not found
        chunk = new Chunk();
        chunk->pos = chunk_data->pos;

        // Chrono chrono;
        const std::lock_guard<std::shared_mutex> lock(chunks_mutex); // TODO: This is where the program waits the most
                                                                    // How to fix: separate chunks and their mesh, so we can have different mutex for data and rendering
                                                                    // Can also just optimize rendering as a temporary solution
        // chrono.log();
        chunks[chunk_data->pos] = chunk;
    } else { // if found
        uint8_t hash_existing_chunk = hashBlocks((uint8_t*)chunk_data->blocks);
        uint8_t hash_new_chunk = hashBlocks((uint8_t*)it->second->blocks);

        if (hash_existing_chunk == hash_new_chunk) {
            return nullptr;
        }

        chunk = it->second;
    }

    memcpy(chunk->blocks, chunk_data->blocks, 4096 * sizeof(uint8_t));

    return chunk;
}

Chunk* World::getChunk(const glm::ivec3& pos) const
{
    const std::shared_lock<std::shared_mutex> lock(chunks_mutex);

    auto it = chunks.find(pos);
    if (it != chunks.end())
        return it->second;
    return nullptr;
}
