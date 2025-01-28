#pragma once

#include <cstdint>
#include <stack>
#include <mutex>

typedef unsigned int GLuint;

typedef struct {
    int32_t start; // bytes;
    int32_t size; // bytes;
    int32_t id;

    // bool isValid() const { return id != -1; }
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
    void deallocate(int32_t id);
    // BufferSlot updateAllocation(int32_t id, uint32_t size, const void * data);

    GLuint getBufferObject() const { return _buffer; };
    int getFreeSlotsCount() const { return _free_slots.size(); };
    int getMaxSlotsCount() const { return _max_slots; };

private:
    const char *_name;
    const size_t _slot_size;
    const size_t _max_slots;

    GLuint _buffer;
    std::stack<int32_t> _free_slots;
    std::mutex _mutex;
};
