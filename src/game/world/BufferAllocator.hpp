#pragma once

#include <glad/gl.h>

#include <sys/types.h>
#include <stack>

typedef struct {
    int32_t start; // startByte;
    int32_t size; // sizeBytes;
    int32_t id;
} BufferSlot;

constexpr BufferSlot invalid_buffer_slot = {-1, -1, -1};

typedef struct {
    uint32_t count;
    uint32_t instanceCount;
    uint32_t firstIndex;
    int32_t  baseVertex;
    uint32_t baseInstance;
} DrawElementsIndirectCommand;

// static const int BUFFER_SIZE = 5e8; // 500 mb
// static const int MAX_DRAW_COMMANDS = 50'000;

class BufferAllocator {
public:
    BufferAllocator(const char* name, uint32_t slot_size, uint32_t max_slots);

    // size in bytes
    BufferSlot allocate(uint32_t size, const void * data);
    void deallocate(int id);
    BufferSlot updateAllocation(uint32_t id, uint32_t size, const void * data);

    GLuint getBufferObject();

private:
    const char *_name;
    uint32_t _slot_size;
    uint32_t _max_slots;

    GLuint _buffer;

    std::stack<uint32_t> _free_slots;
};
