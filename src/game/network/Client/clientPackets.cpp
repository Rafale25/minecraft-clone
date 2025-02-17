#include "Client.hpp"

#include <algorithm>
#include "World.hpp"

Packet::Server::ChunkPacket* readChunkPacket(ByteBuffer& buffer)
{
    auto* chunk_data = new Packet::Server::ChunkPacket;

    int32_t x = buffer.getInt();
    int32_t y = buffer.getInt();
    int32_t z = buffer.getInt();

    chunk_data->pos = glm::ivec3(x, y, z) / 16;

    for (int32_t i = 0 ; i < 16*16*16 ; ++i) {
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
    chunk_data->pos = glm::ivec3(x, y, z) / 16;
    memset(chunk_data->blocks, blockType, 16*16*16);

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
    Client::instance().client_id = buffer.getInt();
}

void Client::decodePacketAddEntity(ByteBuffer buffer)
{
    Client& client = Client::instance();

    auto [id, pos, yaw, pitch, name] = readAddEntityPacket(buffer);

    client.task_queue.push_safe([=, name=std::string(name)]() { // wtf is this syntax
        Entity e{id, pos};
        // e.transform.rotation.y = yaw;
        // e.transform.rotation.x = pitch;
        e.name = name;
        World::instance().addEntity(e);
    } );
}

void Client::decodePacketRemoveEntity(ByteBuffer buffer)
{
    Client& client = Client::instance();

    int32_t entity_id = buffer.getInt();
    client.task_queue.push_safe([=]() {
        World::instance().removeEntity(entity_id);
    });
}

void Client::decodePacketUpdateEntity(ByteBuffer buffer)
{
    Client& client = Client::instance();

    auto [entity_id, pos, yaw, pitch] = readUpdateEntityPacket(buffer);

    client.task_queue.push_safe([=]() {
        World::instance().setEntityTransform(entity_id, pos, yaw, pitch);
    } );
}

void Client::decodePacketChunk(ByteBuffer buffer)
{
    Client& client = Client::instance();

    auto* chunk_data = readChunkPacket(buffer);

    const std::lock_guard<std::mutex> lock(client.new_chunks_mutex);

    // Replace chunk if already in new chunk list to reduce charge on mainthread //
    auto it = std::find_if(client.new_chunks.begin(), client.new_chunks.end(), [&](const auto& chunk){ return chunk->pos == chunk_data->pos; });
    if (it != client.new_chunks.end()) {
        delete *it;
        *it = chunk_data;
    } else {
        client.new_chunks.push_front(chunk_data);
    }
}

void Client::decodePacketMonotypeChunk(ByteBuffer buffer)
{
    Client& client = Client::instance();

    auto* chunk_data = readFullMonoChunkPacket(buffer);
    const std::lock_guard<std::mutex> lock(client.new_chunks_mutex);
    client.new_chunks.push_front(chunk_data);
}

void Client::decodePacketEntityMetadata(ByteBuffer buffer)
{
    Client& client = Client::instance();

    auto [id, name] = readUpdateEntityMetadata(buffer);

    client.task_queue.push_safe([=, name=std::string(name)]() {
        World::instance().setEntityName(id, name);
    } );
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

    client._tchat->push_back(str);
}
