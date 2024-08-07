#pragma once

#include <cstdint>

namespace Packet
{
    namespace Client {
        /* assure data is packed and no padding is used */
        #pragma pack(push, 1)

        struct UpdateBlock {
            uint8_t id;
            uint8_t blockType;
            int x, y, z;
        };

        struct UpdateEntity {
            uint8_t id;
            int x, y, z, yaw, pitch; // float encoded in int
        };

        struct ChatMessage {
            uint8_t id;
            char buffer[4096];
        };

        struct ClientMetadata {
            uint8_t id;
            uint8_t render_distance;
            char name[64];
        };

        #pragma pack(pop)

        enum PacketID {
            PACKET_UPDATE_ENTITY    = 0x00,
            PACKET_EDIT_BLOCK       = 0x01,
            PACKET_EDIT_BLOCK_BULK  = 0x02,
            PACKET_TEXT_MESSAGE     = 0x03,
            PACKET_CLIENT_METADATA  = 0x04,
        };

        // constexpr size_t packet_size[] = {
        //     [PACKET_UPDATE_ENTITY]      = sizeof(UpdateBlock),
        //     [PACKET_EDIT_BLOCK]         = sizeof(UpdateEntity),
        //     [PACKET_EDIT_BLOCK_BULK]    = sizeof(TextMessage),
        //     [PACKET_TEXT_MESSAGE]       = sizeof(UpdateBlock),
        //     [PACKET_CLIENT_METADATA]    = sizeof(UpdateBlock),
        // };
    }
}
