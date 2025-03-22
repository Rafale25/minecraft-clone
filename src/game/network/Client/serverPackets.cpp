#include "Client.hpp"

#include "endianess.h"
#include "byte_manipulation.hpp"

void Client::sendBreakBlockPacket(const glm::ivec3& world_pos)
{
    Packet::Client::UpdateBlock packet = {};

    packet.id = Packet::Client::PACKET_EDIT_BLOCK; // update block //
    packet.blockType = (uint8_t)BlockType::Air;

    packet.x = htobe32(*(uint32_t*)&world_pos.x);
    packet.y = htobe32(*(uint32_t*)&world_pos.y);
    packet.z = htobe32(*(uint32_t*)&world_pos.z);

    sendPacket(&packet, sizeof(packet));
}

void Client::sendBlockBulkEditPacket(const std::vector<std::tuple<glm::ivec3, BlockType>> blocks)
{
    size_t size_in_bytes = sizeof(uint8_t) +
                            sizeof(uint32_t) +
                            blocks.size() * (sizeof(uint8_t) + 3*sizeof(int32_t));

    auto buffer = std::make_unique<uint8_t[]>(size_in_bytes);
    uint8_t *head = &buffer[0];

    // id
    head[0] = Packet::Client::PACKET_EDIT_BLOCK_BULK;
    head += sizeof(uint8_t);

    // blockCount
    putIntBe(head, blocks.size());
    head += sizeof(int32_t);

    for (const auto& [pos, blocktype] : blocks) {
        head[0] = (uint8_t)blocktype;
        head += sizeof(uint8_t);

        putIntBe(head, pos.x);
        head += sizeof(int32_t);

        putIntBe(head, pos.y);
        head += sizeof(int32_t);

        putIntBe(head, pos.z);
        head += sizeof(int32_t);
    }

    sendPacket(buffer.get(), size_in_bytes);
}


void Client::sendBlockBulkEditPacketMonotype(const std::vector<glm::ivec3>& world_pos, BlockType blocktype)
{
    size_t size_in_bytes = sizeof(uint8_t) +
                            sizeof(uint32_t) +
                            world_pos.size() * (sizeof(uint8_t) + 3*sizeof(int32_t));

    auto buffer = std::make_unique<uint8_t[]>(size_in_bytes);
    uint8_t *head = &buffer[0];

    // id
    head[0] = Packet::Client::PACKET_EDIT_BLOCK_BULK;
    head += sizeof(uint8_t);

    // blockCount
    putIntBe(head, world_pos.size());
    head += sizeof(int32_t);

    for (size_t i = 0 ; i < world_pos.size() ; ++i)
    {
        head[0] = (uint8_t)blocktype;
        head += sizeof(uint8_t);

        putIntBe(head, world_pos[i].x);
        head += sizeof(int32_t);

        putIntBe(head, world_pos[i].y);
        head += sizeof(int32_t);

        putIntBe(head, world_pos[i].z);
        head += sizeof(int32_t);
    }

    sendPacket(buffer.get(), size_in_bytes);
}

void Client::sendPlaceBlockPacket(const glm::ivec3& world_pos, BlockType blocktype)
{
    Packet::Client::UpdateBlock packet = {};

    packet.id = Packet::Client::PACKET_EDIT_BLOCK; // update block //
    packet.blockType = (uint8_t)blocktype;
    packet.x = htobe32(*(uint32_t*)&world_pos.x);
    packet.y = htobe32(*(uint32_t*)&world_pos.y);
    packet.z = htobe32(*(uint32_t*)&world_pos.z);

    sendPacket(&packet, sizeof(packet));
}

void Client::sendUpdateEntityPacket(const glm::vec3& pos, float yaw, float pitch)
{
    // if (client_id == -1) return;

    Packet::Client::UpdateEntity packet = {};

    packet.id = Packet::Client::PACKET_UPDATE_ENTITY; // update entity //
    packet.x = htobe32(*(uint32_t*)&pos.x);
    packet.y = htobe32(*(uint32_t*)&pos.y);
    packet.z = htobe32(*(uint32_t*)&pos.z);
    packet.yaw = htobe32(*(uint32_t*)&yaw);
    packet.pitch = htobe32(*(uint32_t*)&pitch);

    sendPacket(&packet, sizeof(packet));
}

void Client::sendChatMessagePacket(const char* buffer)
{
    Packet::Client::ChatMessage packet = {};

    size_t size = strlen(buffer) * sizeof(char);
    assert(size <= sizeof(packet.buffer));

    packet.id = Packet::Client::PACKET_TEXT_MESSAGE;
    memcpy(packet.buffer, buffer, size);

    sendPacket(&packet, sizeof(packet));
}

void Client::sendClientMetadataPacket(int32_t render_distance, std::string name)
{
    Packet::Client::ClientMetadata packet = {};

    packet.id = Packet::Client::PACKET_CLIENT_METADATA;
    packet.render_distance = render_distance;
    memcpy(packet.name, name.c_str(), name.length());

    sendPacket(&packet, sizeof(packet));
}
