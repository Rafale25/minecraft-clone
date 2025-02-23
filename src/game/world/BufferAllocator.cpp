#include "BufferAllocator.hpp"

#include <glad/gl.h>

#include <stdio.h>
#include <cassert>

#define PRINT_ERRORS

BufferAllocator::BufferAllocator(const char* name, uint32_t max_memory):
    _name(name),
    _max_memory(max_memory),
    _available_memory(max_memory)
{
    glCreateBuffers(1, &_buffer);

    if (max_memory > MAX_BUFFER_SIZE) {
        fprintf(stderr, "Error BufferAllocator: %s - Trying to allocated %u which is more than the maximum of %llu\n", name, max_memory, MAX_BUFFER_SIZE);
        abort();
    }

    printf("Buffer size: %u\n", max_memory);

    glNamedBufferStorage(_buffer, max_memory, nullptr, GL_DYNAMIC_STORAGE_BIT);

    _slots.emplace_back(0, max_memory, false);
}

BufferSlot BufferAllocator::allocate(uint32_t size, const void * data) {
    for (auto it = _slots.begin() ; it != _slots.end() ; ++it) {
        if (it->used == true) continue;

        if (it->size == size) {
            it->used = true;

            _available_memory -= size;

            glNamedBufferSubData(
                _buffer,
                it->start,
                it->size,
                data
            );

            return *it;
        }

        if (it->size > size) {
            BufferSlot b {
                .start = it->start,
                .size = size,
                .used = true
            };

            it->start += size;
            it->size -= size;
            it->used = false;

            _slots.insert(it, b);

            _available_memory -= size;

            glNamedBufferSubData(
                _buffer,
                b.start,
                b.size,
                data
            );

            return b;
        }
    }

    return invalid_buffer_slot;
}

void BufferAllocator::deallocate(int32_t id) {
    // NOTE: id is start
    if (id <= -1) return;

    for (auto it = _slots.begin() ; it != _slots.end() ; ++it) {
        if (it->start == id) {
            it->used = false;
            _available_memory += it->size;

            defragmentAt(it);
            break;
        }
    }
}

void BufferAllocator::defragmentAt(const std::list<BufferSlot>::iterator it) {
    auto prev_it = it;
    auto next_it = std::next(it);

    // TODO: need to defragment in both direction

    while (1) {
        if (_slots.size() > 1 && next_it != _slots.end() && next_it->used == false) {
            next_it->start = prev_it->start;
            next_it->size += prev_it->size;

            _slots.erase(prev_it);
        } else {
            break;
        }

        prev_it = next_it;
        next_it = std::next(next_it);
    }
}


// BufferSlot BufferAllocator::allocate(uint32_t size, const void * data) {
//     if (size == 0) {
//         #ifdef PRINT_ERRORS
//         fprintf(stderr, "Error: %s - Trying to allocate size of 0!\n", _name);
//         #endif

//         return invalid_buffer_slot;
//     }

//     if (size > _slot_size) {
//         #ifdef PRINT_ERRORS
//         fprintf(stderr, "Error: %s - Allocate size demanded %u is higher than maximum slot size of %u\n", _name, (uint32_t)size, (uint32_t)_slot_size);
//         #endif

//         return invalid_buffer_slot;
//     }

//     if (_free_slots.size() <= 0) {
//         #ifdef PRINT_ERRORS
//         fprintf(stderr, "Error: %s - No free slot in buffer\n", _name);
//         #endif

//         return invalid_buffer_slot;
//     }

//     int32_t id = _free_slots.top();
//     _free_slots.pop();

//     assert(id >= 0 && "Error: bufferslot id is invalid!");

//     #ifdef PRINT_ERRORS
//     // printf("Info: %s - Allocating %d - ID %d == %ld\n", _name, size, id, id * _slot_size + size);
//     #endif

//     BufferSlot b = {
//         .start = (int32_t) (id * _slot_size),
//         .size = (int32_t) size,
//         .id = (int32_t) id
//     };

//     glNamedBufferSubData(
//         _buffer,
//         b.start,
//         size,
//         data
//     );

//     return b;
// }

// void BufferAllocator::deallocate(int32_t id) {
//     if (id <= -1) {
//         #ifdef PRINT_ERRORS
//         // printf("Error: %s - Tried to deallocated indalid id %d!\n", _name, id);
//         #endif
//         return;
//     }

//     _free_slots.push(id);
// }
