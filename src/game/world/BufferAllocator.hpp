#pragma once

#include <cstdint>
#include <map>
#include <vector>
#include <stack>
#include <list>
#include <tuple>
#include <mutex>

typedef unsigned int GLuint;

/*
start, size, used

slots [ {0, MAX_SIZE, false} ]

allocate(64)
slots [ {0, 64, true}, {64, MAX_SIZE, false} ]

allocate(64)
slots [ {0, 64, true}, {64, 64, true}, {128, MAX_SIZE, false} ]

allocate(64)
slots [ {0, 64, true}, {64, 64, true}, {128, 64, true}, {192, MAX_SIZE, false} ]

deallocate(id(1))
slots [ {0, 64, true}, {64, 64, false}, {128, 64, true}, {192, MAX_SIZE, false} ]

deallocate(id(0))
slots [ {0, 128, false}, {128, 64, true}, {192, MAX_SIZE, false} ]
*/

struct BufferSlot {
    int32_t start; // bytes;
    int32_t size; // bytes;

    bool used = false;

    // bool is_valid;
    std::list<BufferSlot>::iterator it{nullptr};
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

    BufferSlot allocate(uint32_t size, const void * data);
    // void deallocate(int32_t id);
    void deallocate(const BufferSlot& slot);

    GLuint getBufferObject() const { return _buffer; };
    int32_t getMaxMemory() const { return _max_memory; };
    int32_t getAvailableMemory() const { return _available_memory; };
    int32_t getSlotCount() const { return _slots.size(); };

private:
    void defragmentAt(const std::list<BufferSlot>::iterator it);

private:
    const char* _name;
    const size_t _max_memory;
    size_t _available_memory;

    GLuint _buffer;

    // size // std::vector<starts>
    // std::map<int32_t, std::vector<int32_t>> _free_slot_of_size;

    // size // std::vector<reference to a BufferSlot in _slots>
    // std::map<int32_t, std::vector<BufferSlot&>> _free_slot_of_size;
    std::map<int32_t, std::vector<std::list<BufferSlot>::iterator>> _free_slot_of_size;

    std::list<BufferSlot> _slots;

    // dod::slot_map<BufferSlot> _slots;
    // NEED TO IMPLEMENT A LINKED LIST

    // CANT USE SLOT_MAP, cannot insert at specific place like a linked list
    // SLOT_MAP is just a better unordered_map for O(1) insert, remove, and pretty fast iteration


    std::mutex _mutex;
};
