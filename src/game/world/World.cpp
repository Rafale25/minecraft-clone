#include "World.hpp"
#include "Chunk.hpp"
#include "BlockRaycastHit.hpp"
#include <cstring>
#include <mutex>

World::World()
{
}

Entity* World::getEntity(int32_t id)
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
    const float smoothness = 0.2f;
    for (auto& entity : entities)
    {
        entity.smooth_transform.position = glm::mix(entity.smooth_transform.position, entity.transform.position, smoothness);
        entity.smooth_transform.rotation = glm::slerp(entity.smooth_transform.rotation, entity.transform.rotation, smoothness);
    }
}

void World::addEntity(Entity e)
{
    entities.push_back(e);
}

void World::removeEntity(int32_t id)
{
    for (size_t i = 0 ; i < entities.size() ; ++i)
    {
        if (entities[i].id == id) {
            entities.erase(entities.begin() + i);
            // delete entity ?
        }
    }
}

void World::setEntityTransform(int32_t id, const glm::vec3& pos, float yaw, float pitch)
{
    Entity* e = getEntity(id);
    if (e == nullptr) return;
    e->transform.position = pos;
    e->transform.rotation = glm::quat(glm::vec3(-pitch, -yaw, 0.0f));
}

void World::setEntityName(int32_t id, std::string name)
{
    Entity* e = getEntity(id);
    if (e == nullptr) return;
    e->name = name;
}

BlockType World::getBlock(const glm::vec3& posf) const
{
    return getBlock(glm::ivec3(glm::floor(posf)));
}

BlockType World::getBlock(const glm::ivec3& pos) const
{
    glm::ivec3 chunk_pos = glm::floor(glm::vec3(pos) / CHUNK_SIZEF);
    glm::ivec3 local_pos = {pos.x % CHUNK_SIZE, pos.y % CHUNK_SIZE, pos.z % CHUNK_SIZE};
    if (local_pos.x < 0) local_pos.x += CHUNK_SIZE;
    if (local_pos.y < 0) local_pos.y += CHUNK_SIZE;
    if (local_pos.z < 0) local_pos.z += CHUNK_SIZE;

    // printf("chunck pos: %d %d %d\n", chunk_pos.x, chunk_pos.y, chunk_pos.z);
    // printf("local_pos: %d %d %d\n", local_pos.x, local_pos.y, local_pos.z);
    auto it = chunks.find(chunk_pos);
    if (it == chunks.end()) return BlockType::Air; // chunk doesn't exist //

    int32_t index = Chunk::XYZtoIndex(local_pos.x, local_pos.y, local_pos.z);
    return it->second->blocks[index];
}

BlockRaycastHit World::blockRaycast(const glm::vec3& origin, const glm::vec3& direction, float maxDistance) const
{
    const glm::vec3 rayPos = origin;
    const glm::vec3 deltaDist = abs(glm::vec3(glm::length(direction)) / direction);
	const glm::vec3 rayStep = sign(direction);

    glm::vec3 mapPos = glm::ivec3(glm::floor(rayPos));

    glm::vec3 sideDist = (sign(direction) * (mapPos - rayPos) + (sign(direction) * 0.5f) + 0.5f) * deltaDist;
	glm::vec3 mask = {0.0f, 0.0f, 0.0f};
    glm::vec3 normal = {0.0f, 0.0f, 0.0f};
    glm::vec3 intersection_pos = {0.0f, 0.0f, 0.0f};
    float intersection_distance = 0.0f;

    #define MAX_ITERATION 500
	for (int32_t i = 0; i < MAX_ITERATION; i++) {
        auto block = getBlock(mapPos);
        normal = -mask * rayStep;

        intersection_distance = glm::length(mask * (sideDist - deltaDist));
        intersection_pos = rayPos + intersection_distance * direction;

        if (block != BlockType::Air) {
            return {true, block, glm::floor(mapPos), intersection_pos, normal};
        };

        mask = glm::step(sideDist, glm::vec3(sideDist.y, sideDist.z, sideDist.x)) * glm::step(sideDist, glm::vec3(sideDist.z, sideDist.x, sideDist.y));
		sideDist += mask * deltaDist;
		mapPos += mask * rayStep;

        if (intersection_distance > maxDistance) {
            break;
        }
	}

    return {false, BlockType::Air, glm::floor(mapPos), intersection_pos, normal};
}

uint32_t hashBlocks(const uint8_t* values) {
    uint32_t h = 1;

    for (int32_t i = 0 ; i < CHUNK_BLOCK_COUNT ; ++i) {
        h *= (1779033703 + 2*(uint32_t)values[i]);
    }

    return h;
}

Chunk* World::setChunk(const glm::ivec3& pos, const BlockType* blocks)
{
    Chunk* chunk = nullptr;

    // Chrono chrono;
    const std::lock_guard<std::shared_mutex> lock(chunks_mutex);// TODO: This is where the program waits the most
                                                                // How to fix: separate chunks and their mesh, so we can have different mutex for data and rendering
                                                                // Can also just optimize rendering as a temporary solution
    // chrono.log();

    auto it = chunks.find(pos);

    if (it == chunks.end()) { // if not found
        chunk = new Chunk();
        chunk->pos = pos;

        chunks[pos] = chunk;
    } else { // if found
        uint32_t hash_existing_chunk = hashBlocks((uint8_t*)blocks);
        uint32_t hash_new_chunk = hashBlocks((uint8_t*)it->second->blocks);

        if (hash_existing_chunk == hash_new_chunk) {
            return nullptr;
        }

        chunk = it->second;
    }

    // TODO: do the memcpy outside of the mutex lock
    memcpy(chunk->blocks, blocks, CHUNK_BLOCK_COUNT * sizeof(uint8_t));

    return chunk;
}

void World::deleteChunk(const glm::ivec3 &pos) {
    //NOTE: might need to use .find() in case pos doesn't exist
    Chunk* chunk = chunks.at(pos);
    if (chunk == nullptr) return;

    delete chunk;
    chunks.erase(pos);
}

Chunk* World::getChunk(const glm::ivec3& pos) const
{
    const std::shared_lock<std::shared_mutex> lock(chunks_mutex);

    auto it = chunks.find(pos);
    if (it != chunks.end())
        return it->second;
    return nullptr;
}

Chunk* World::getChunkUnsafe(const glm::ivec3& pos) const
{
    auto it = chunks.find(pos);
    if (it != chunks.end())
        return it->second;
    return nullptr;
}
