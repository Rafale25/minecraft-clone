#include "Client.hpp"
#include "World.hpp"
#include "constants.hpp"
#include <algorithm>

Packet::Server::ChunkPacket* readChunkPacket(ByteBuffer& buffer)
{
    auto* chunk_data = new Packet::Server::ChunkPacket;

    int32_t x = buffer.getInt();
    int32_t y = buffer.getInt();
    int32_t z = buffer.getInt();

    chunk_data->pos = glm::ivec3(x, y, z) / CHUNK_SIZE;

    for (int32_t i = 0 ; i < CHUNK_BLOCK_COUNT ; ++i) {
        uint8_t byte = buffer.get();

        /* convert to BlackoutBurst indexing -_- */
        // int32_t x = i % 16;
        // int32_t y = (i / 16) % 16;
        // int32_t z = i / (16 * 16);
        // int32_t index = x * 16*16 + y * 16 + z;
        // chunk.blocks[index] = (BlockType)byte;

        chunk_data->blocks[i] = (BlockType)byte;
    }

    return chunk_data;
}

Packet::Server::ChunkPacket* readFullMonoChunkPacket(ByteBuffer buffer)
{
    int32_t x = buffer.getInt();
    int32_t y = buffer.getInt();
    int32_t z = buffer.getInt();
    uint8_t blockType = buffer.get();

    auto *chunk_data = new Packet::Server::ChunkPacket;
    chunk_data->pos = glm::ivec3(x, y, z) / CHUNK_SIZE;
    memset(chunk_data->blocks, blockType, CHUNK_BLOCK_COUNT);

    return chunk_data;
}

Packet::Server::AddEntity readAddEntityPacket(ByteBuffer buffer)
{
    Packet::Server::AddEntity packet = {};

    packet.id = buffer.getInt();
    packet.position.x = buffer.getFloat();
    packet.position.y = buffer.getFloat();
    packet.position.z = buffer.getFloat();
    packet.yaw = buffer.getFloat();
    packet.pitch = buffer.getFloat();
    buffer.getN((uint8_t*)packet.name, 64);

    return packet;
}

Packet::Server::UpdateEntity readUpdateEntityPacket(ByteBuffer buffer)
{
    Packet::Server::UpdateEntity packet = {};

    packet.entity_id = buffer.getInt();
    packet.position.x = buffer.getFloat();
    packet.position.y = buffer.getFloat();
    packet.position.z = buffer.getFloat();
    packet.yaw = buffer.getFloat();
    packet.pitch = buffer.getFloat();

    return packet;
}

Packet::Server::UpdateEntityMetadata readUpdateEntityMetadata(ByteBuffer buffer)
{
    Packet::Server::UpdateEntityMetadata packet = {};

    packet.entity_id = buffer.getInt();
    buffer.getN((uint8_t*)packet.name, 64);

    return packet;
}

void Client::decodePacketIdentification(ByteBuffer buffer)
{
    Client::instance().m_clientId = buffer.getInt();
}

void Client::decodePacketAddEntity(ByteBuffer buffer)
{
    Client& client = Client::instance();

    Packet::Server::AddEntity packet = readAddEntityPacket(buffer);

    client.m_taskQueue.push_safe([=]() {
        Entity e{packet.id, packet.position};
        // e.transform.rotation.y = yaw;
        // e.transform.rotation.x = pitch;
        e.name = std::string(packet.name);
        World::instance().addEntity(e);
    } );
}

void Client::decodePacketRemoveEntity(ByteBuffer buffer)
{
    Client& client = Client::instance();

    int32_t entity_id = buffer.getInt();
    client.m_taskQueue.push_safe([=]() {
        World::instance().removeEntity(entity_id);
    });
}

void Client::decodePacketUpdateEntity(ByteBuffer buffer)
{
    Client& client = Client::instance();

    Packet::Server::UpdateEntity packet = readUpdateEntityPacket(buffer);

    client.m_taskQueue.push_safe([=]() {
        World::instance().setEntityTransform(packet.entity_id, packet.position, packet.yaw, packet.pitch);
    } );
}

void Client::decodePacketChunk(ByteBuffer buffer)
{
    Client& client = Client::instance();

    auto* chunk_data = readChunkPacket(buffer);

    const std::lock_guard<std::mutex> lock(client.m_newChunksMutex);

    // Replace chunk if already in new chunk list to reduce charge on mainthread //
    auto it = std::find_if(client.m_newChunks.begin(), client.m_newChunks.end(), [&](const auto& chunk){ return chunk->pos == chunk_data->pos; });
    if (it != client.m_newChunks.end()) {
        delete *it;
        *it = chunk_data;
    } else {
        client.m_newChunks.push_front(chunk_data);
    }
}

void Client::decodePacketMonotypeChunk(ByteBuffer buffer)
{
    Client& client = Client::instance();

    auto* chunk_data = readFullMonoChunkPacket(buffer);
    const std::lock_guard<std::mutex> lock(client.m_newChunksMutex);
    client.m_newChunks.push_front(chunk_data);
}

void Client::decodePacketEntityMetadata(ByteBuffer buffer)
{
    Client& client = Client::instance();

    Packet::Server::UpdateEntityMetadata packet = readUpdateEntityMetadata(buffer);

    client.m_taskQueue.push_safe([=]() {
        World::instance().setEntityName(packet.entity_id, std::string(packet.name));
    });
}

void Client::decodePacketChatMessage(ByteBuffer buffer)
{
    Client& client = Client::instance();

    std::string str = std::string((char*)buffer.getPtr(), 4096);

    // Removes '&[]' from message //
    size_t index = 0;
    while (true) {
        index = str.find("&", index);
        if (index == std::string::npos) break;
        str.replace(index, 2, "");
        index += 2;
    }
    // --

    client.m_tchat->push_back(str);
}
