#pragma once

#include <cstdint>
#include <map>
#include <vector>
#include <list>

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

struct DrawArraysIndirectCommand {
    uint32_t count;
    uint32_t instanceCount;
    uint32_t first;
    uint32_t baseInstance;
};

// NOTE: Crash when max size exceed int32_t max
static constexpr uint64_t MAX_BUFFER_SIZE = 2'147'483'647; // 2^31-1 ~2.147Go

class BufferAllocator {
public:
    BufferAllocator(const char* name, uint32_t max_memory);

    BufferSlot allocate(int32_t size, const void * data);
    void deallocate(const BufferSlot& slot);

    GLuint getBufferObject() const { return m_buffer; };
    int32_t getMaxMemory() const { return m_maxMemory; };
    int32_t getAvailableMemory() const { return m_availableMemory; };
    int32_t getSlotCount() const { return m_slots.size(); };

// private:
    const char* m_name;
    const size_t m_maxMemory;
    size_t m_availableMemory;

    GLuint m_buffer;

    std::map<int32_t, std::vector<std::list<BufferSlot>::iterator>> m_freeSlotOfSize;
    std::list<BufferSlot> m_slots;
};
