#pragma once

#include <cstdint>
#include <map>
#include <vector>
#include <stack>
#include <list>
#include <tuple>
#include <mutex>

typedef unsigned int GLuint;

struct BufferSlot {
    int32_t start; // bytes;
    int32_t size; // bytes;
    bool used = false;
    std::list<BufferSlot>::iterator it{};
};

const BufferSlot invalid_buffer_slot = {.start=-1, .size=-1, .used=false};

struct DrawElementsIndirectCommand {
    uint32_t count;
    uint32_t instanceCount;
    uint32_t firstIndex;
    int32_t  baseVertex;
    uint32_t baseInstance;
};

// NOTE: Crash when max size exceed int32_t max
static constexpr uint64_t MAX_BUFFER_SIZE = 20e8; // 2000 mb

class BufferAllocator {
public:
    BufferAllocator(const char* name, uint32_t max_memory);

    BufferSlot allocate(int32_t size, const void * data);
    void deallocate(const BufferSlot& slot);

    GLuint getBufferObject() const { return _buffer; };
    int32_t getMaxMemory() const { return _max_memory; };
    int32_t getAvailableMemory() const { return _available_memory; };
    int32_t getSlotCount() const { return _slots.size(); };

private:
    const char* _name;
    const size_t _max_memory;
    size_t _available_memory;

    GLuint _buffer;

    std::map<int32_t, std::vector<std::list<BufferSlot>::iterator>> _free_slot_of_size;
    std::list<BufferSlot> _slots;
};
