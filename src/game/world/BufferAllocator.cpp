#include "BufferAllocator.hpp"

#include <glad/gl.h>

#include <stdio.h>
#include <cassert>
#include <clock.h>

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
    _free_slot_of_size[max_memory].push_back(--_slots.end()); // iterator to last element
}

BufferSlot BufferAllocator::allocate(uint32_t size, const void * data) {
    SimpleProfiler::instance().start("BufferAllocator::allocate");

    const auto it = _free_slot_of_size.equal_range(size).first;

    // printf("size found: %d\n", it->first);

    if (it == _free_slot_of_size.end()) {
        printf("ERROR: No slot of size bigger or equal to %d available\n", size);
        return invalid_buffer_slot;
    } else {
        auto& free_slots = it->second; // vector of iterator
        const int32_t slots_size = it->first;

        if (free_slots.size() <= 0) {
            printf("ERROR: THIS SHOULD NOT HAPPEN - Found size %d for requested size of %d - NO SLOTS INSIDE\n", it->first, size);
            printf("ABORT\n");
            abort();
            return invalid_buffer_slot;
        }

        auto slot_it = free_slots.back();
        BufferSlot& slot = *slot_it;
        free_slots.pop_back();

        if (free_slots.size() == 0) {
            _free_slot_of_size.erase(it->first);
        }

        if (slots_size == size) {
            // printf("SLOT SIZE EQUAL\n");

            slot.used = true;

            _available_memory -= size;

            glNamedBufferSubData(
                _buffer,
                slot.start,
                slot.size,
                data
            );

            return slot;
        }

        if (slots_size > size) {
            // printf("SLOT SIZE LARGER\n");

            BufferSlot b {
                .start = slot.start,
                .size = size,
                .used = true
            };

            slot.start += size;
            slot.size -= size;
            slot.used = false;

            auto inserted_it = _slots.insert(slot_it, b);
            inserted_it->it = inserted_it;
            b.it = inserted_it;

            _free_slot_of_size[slot.size].push_back(slot_it);

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

    // SimpleProfiler::instance().stop("BufferAllocator::allocate", false);
    return invalid_buffer_slot;
}

void BufferAllocator::deallocate(const BufferSlot& slot) {
    // NOTE: id is start
    if (slot.start <= -1 || slot.size <= -1) return;

    // std::cout << std::distance(slot.it, _slots.begin()) << std::endl;

    slot.it->used = false;
    // printf("[deallocate] size: %d, start: %d\n", slot.size, slot.start);

    _available_memory += slot.it->size;

    _free_slot_of_size[slot.size].push_back(slot.it);

    // HOW DO I DEALLOCATE ?????? WTF I NEED TO RETHING THAT
    // I need to set slot.used to false
    // then I need t

    // defragmentAt(slot.it);
    // _available_memory += slot.it->size;

    // for (auto it = _slots.begin() ; it != _slots.end() ; ++it) {
    //     if (it->start == id) {
    //         it->used = false;
    //         _available_memory += it->size;

    //         defragmentAt(it);
    //         break;
    //     }
    // }
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
