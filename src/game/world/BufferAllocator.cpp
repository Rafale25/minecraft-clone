// #include <stdio.h>
#include <cassert>
#include <algorithm>
#include <glad/gl.h>
#include "BufferAllocator.hpp"
#include "Logger.hpp"

#define PRINT_ERRORS

BufferAllocator::BufferAllocator(const char* name, uint32_t max_memory):
    _name(name),
    _max_memory(max_memory),
    _available_memory(max_memory)
{
    glCreateBuffers(1, &_buffer);

    if (max_memory > MAX_BUFFER_SIZE) {
        fprintf(stderr, "Error BufferAllocator: %s - Trying to allocated %u which is more than the maximum of %lu\n", name, max_memory,  (unsigned long int)(MAX_BUFFER_SIZE));
        abort();
    }

    logI("[BufferAllocator] Allocated size: {}", max_memory);

    glNamedBufferStorage(_buffer, max_memory, nullptr, GL_DYNAMIC_STORAGE_BIT);

    const auto it = _slots.insert(_slots.end(), {.start=0, .size=(int32_t)max_memory, .used=false});
    it->it = it; // assign first element its own iterator

    _free_slot_of_size[max_memory].push_back(--_slots.end()); // iterator to last element
}

BufferSlot BufferAllocator::allocate(int32_t size, const void * data) {
    // SimpleProfiler::instance().start("BufferAllocator::allocate");
    // defer SimpleProfiler::instance().stop("BufferAllocator::allocate");

    const auto it = _free_slot_of_size.equal_range(size).first;

    if (it == _free_slot_of_size.end()) {
        logE("No slot of size bigger or equal to {} available", size);
        return invalid_buffer_slot;
    } else {
        auto& free_slots = it->second; // vector of iterator
        const int32_t slots_size = it->first;

        if (free_slots.size() <= 0) {
            logF("THIS SHOULD NOT HAPPEN - Found size {} for requested size of {} - NO SLOTS INSIDE", it->first, size);
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

    // SimpleProfiler::instance().stop("BufferAllocator::allocate");
    return invalid_buffer_slot;
}

void BufferAllocator::deallocate(const BufferSlot& slot) {
    // NOTE: id is start
    if (slot.start <= -1 || slot.size <= -1 || slot.used == false) return;

    slot.it->used = false;
    // printf("[deallocate] size: %d, start: %d\n", slot.size, slot.start);

    _available_memory += slot.it->size;

    if (slot.it != _slots.begin()) {
        auto prev_it = std::prev(slot.it);

        if (prev_it->used == false) {
            slot.it->start = prev_it->start;
            slot.it->size += prev_it->size;

            auto& free_slots = _free_slot_of_size.at(prev_it->size);

            auto it = std::find_if(free_slots.begin(), free_slots.end(), [&](const std::list<BufferSlot>::iterator& slot_it){ return slot_it == prev_it; });
            free_slots.erase(it);
            if (free_slots.size() == 0) {
                _free_slot_of_size.erase(prev_it->size);
            }
            _slots.erase(prev_it); // NOTE: important to erase at the end because it's basically removing itself
        }
    }

    auto next_it = std::next(slot.it);
    if (next_it != _slots.end() && next_it->used == false) {
        slot.it->size += next_it->size;

        auto& free_slots = _free_slot_of_size.at(next_it->size);

        auto it = std::find_if(free_slots.begin(), free_slots.end(), [&](const std::list<BufferSlot>::iterator& slot_it){ return slot_it == next_it; });
        free_slots.erase(it);
        if (free_slots.size() == 0) {
            _free_slot_of_size.erase(next_it->size);
        }
        _slots.erase(next_it);

    }

    _free_slot_of_size[slot.it->size].push_back(slot.it);
}
