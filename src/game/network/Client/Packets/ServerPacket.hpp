#pragma once

#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_int3.hpp>

#include "enums.hpp"
#include "constants.hpp"

namespace Packet
{
    namespace Server {

/* assure data is packed and no padding is used */
#pragma pack(push, 1)

        struct Identification {
            int32_t client_id;
        };

        struct AddEntity {
            int32_t id;
            glm::vec3 position;
            float yaw;
            float pitch;
            char name[64];
        };

        struct RemoveEntity {
            int32_t entity_id;
        };

        struct UpdateEntity {
            int32_t entity_id;
            glm::vec3 position;
            float yaw;
            float pitch;
        };

        struct ChunkPacket {
            glm::ivec3 pos;
            BlockType blocks[CHUNK_BLOCK_COUNT];
        };

        struct MonoChunkPacket {
            glm::ivec3 pos;
            BlockType block;
        };

        struct ChatMessage {
            uint8_t msg[4096];
        };

        struct UpdateEntityMetadata {
            int32_t entity_id;
            char name[64];
        };

#pragma pack(pop)

    }
}
