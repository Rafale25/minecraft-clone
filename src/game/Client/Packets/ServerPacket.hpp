#pragma once

#include "glm/glm.hpp"
#include "enums.hpp"

namespace Packet
{
    namespace Server {
        struct ChunkPacket {
            glm::ivec3 pos;
            BlockType blocks[4096];
        };

        struct UpdateEntityClientPacket {
            int id;
            glm::vec3 position;
            float yaw;
            float pitch;
        };

        struct AddEntityClientPacket {
            int id;
            glm::vec3 position;
            float yaw;
            float pitch;
            char name[64];
        };
    }
}
