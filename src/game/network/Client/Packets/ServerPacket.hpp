#pragma once

#include "glm/glm.hpp"
#include "enums.hpp"

namespace Packet
{
    namespace Server {

/* assure data is packed and no padding is used */
#pragma pack(push, 1)

        struct Identification {
            int client_id;
        };

        struct AddEntity {
            int id;
            glm::vec3 position;
            float yaw;
            float pitch;
            char name[64];
        };

        struct RemoveEntity {
            int entity_id;
        };

        struct UpdateEntity {
            int entity_id;
            glm::vec3 position;
            float yaw;
            float pitch;
        };

        struct ChunkPacket {
            glm::ivec3 pos;
            BlockType blocks[4096];
        };

        struct MonoChunkPacket {
            glm::ivec3 pos;
            BlockType block;
        };

        struct ChatMessage {
            uint8_t msg[4096];
        };

        struct UpdateEntityMetadata {
            int entity_id;
            char name[64];
        };

#pragma pack(pop)

    }
}
