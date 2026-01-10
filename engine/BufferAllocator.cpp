#include "BufferAllocator.hpp"
#include "Logger.hpp"
#include <glad/gl.h>
#include <cassert>
#include <algorithm>

#define PRINT_ERRORS

BufferAllocator::BufferAllocator(const char* name, uint32_t max_memory):
    m_name(name),
    m_maxMemory(max_memory),
    m_availableMemory(max_memory)
{
    glCreateBuffers(1, &m_buffer);

    if (max_memory > MAX_BUFFER_SIZE) {
        fprintf(stderr, "Error BufferAllocator: %s - Trying to allocated %u which is more than the maximum of %lu\n", name, max_memory,  (unsigned long int)(MAX_BUFFER_SIZE));
        abort();
    }

    logI("[BufferAllocator] Allocated size: {}", max_memory);

    glNamedBufferStorage(m_buffer, max_memory, nullptr, GL_DYNAMIC_STORAGE_BIT);

    const auto it = m_slots.insert(m_slots.end(), {.start=0, .size=(int32_t)max_memory, .used=false});
    it->it = it; // assign first element its own iterator

    m_freeSlotOfSize[max_memory].push_back(--m_slots.end()); // iterator to last element
}

BufferSlot BufferAllocator::allocate(int32_t size, const void * data) {
    // SimpleProfiler::instance().start("BufferAllocator::allocate");

    const auto it = m_freeSlotOfSize.equal_range(size).first;

    if (it == m_freeSlotOfSize.end()) {
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
            m_freeSlotOfSize.erase(it->first);
        }

        if (slots_size == size) {
            slot.used = true;

            m_availableMemory -= size;

            glNamedBufferSubData(
                m_buffer,
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

            auto inserted_it = m_slots.insert(slot_it, b);
            inserted_it->it = inserted_it;
            b.it = inserted_it;

            m_freeSlotOfSize[slot.size].push_back(slot_it);

            m_availableMemory -= size;

            glNamedBufferSubData(
                m_buffer,
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

    m_availableMemory += slot.it->size;

    if (slot.it != m_slots.begin()) {
        auto prev_it = std::prev(slot.it);

        if (prev_it->used == false) {
            slot.it->start = prev_it->start;
            slot.it->size += prev_it->size;

            auto& free_slots = m_freeSlotOfSize.at(prev_it->size);

            auto it = std::find_if(free_slots.begin(), free_slots.end(), [&](const std::list<BufferSlot>::iterator& slot_it){ return slot_it == prev_it; });
            free_slots.erase(it);
            if (free_slots.size() == 0) {
                m_freeSlotOfSize.erase(prev_it->size);
            }
            m_slots.erase(prev_it); // NOTE: important to erase at the end because it's basically removing itself
        }
    }

    auto next_it = std::next(slot.it);
    if (next_it != m_slots.end() && next_it->used == false) {
        slot.it->size += next_it->size;

        auto& free_slots = m_freeSlotOfSize.at(next_it->size);

        auto it = std::find_if(free_slots.begin(), free_slots.end(), [&](const std::list<BufferSlot>::iterator& slot_it){ return slot_it == next_it; });
        free_slots.erase(it);
        if (free_slots.size() == 0) {
            m_freeSlotOfSize.erase(next_it->size);
        }
        m_slots.erase(next_it);

    }

    m_freeSlotOfSize[slot.it->size].push_back(slot.it);
}
