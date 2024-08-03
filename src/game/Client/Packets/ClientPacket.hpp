#pragma once

#include <cstdint>

namespace Packet
{
    namespace Client {
        struct __attribute__ ((packed)) updateBlockServerPacket {
            uint8_t id;
            uint8_t blockType;
            int x, y, z;
        };

        struct __attribute__ ((packed)) updateEntityServerPacket {
            uint8_t id;
            int x, y, z, yaw, pitch; // float encoded in int
        };

        struct __attribute__ ((packed)) sendTextMessageServerPacket {
            uint8_t id;
            char buffer[4096];
        };

        struct __attribute__ ((packed)) ClientMetadata {
            uint8_t id;
            uint8_t render_distance;
            char name[64];
        };
    }
}
