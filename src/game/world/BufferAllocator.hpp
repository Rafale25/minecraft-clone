#pragma once

#include <glad/gl.h>

#include <sys/types.h>
#include <stack>

typedef struct {
    int32_t start; // bytes;
    int32_t size; // bytes;
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

// NOTE: Crash when max size exceed int32_t max
static constexpr uint64_t MAX_BUFFER_SIZE = 20e8; // 2000 mb

class BufferAllocator {
public:
    BufferAllocator(const char* name, uint32_t slot_size, uint32_t max_slots);

    BufferSlot allocate(uint32_t size, const void * data);
    void deallocate(int id);
    BufferSlot updateAllocation(uint32_t id, uint32_t size, const void * data);

    GLuint getBufferObject();

private:
    const char *_name;
    size_t _slot_size;
    size_t _max_slots;

    GLuint _buffer;

    std::stack<uint32_t> _free_slots;
};
