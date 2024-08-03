#pragma once

#include <cstdint>

namespace Packet
{
    namespace Client {

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

        struct SendTextMessage {
            uint8_t id;
            char buffer[4096];
        };

        struct ClientMetadata {
            uint8_t id;
            uint8_t render_distance;
            char name[64];
        };

#pragma pack(pop)

    }
}
