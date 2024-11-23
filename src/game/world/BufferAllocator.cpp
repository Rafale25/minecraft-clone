#include "BufferAllocator.hpp"

#include <glad/gl.h>

#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#define PRINT_ERRORS

BufferAllocator::BufferAllocator(const char* name, uint32_t slot_size, uint32_t max_slots): _name(name), _slot_size(slot_size), _max_slots(max_slots) {
    printf("Slotsize: %d\n", slot_size);
    printf("MAxSlot: %d\n", max_slots);

    glCreateBuffers(1, &_buffer);
    const uint64_t buffer_size = slot_size * max_slots;

    if (buffer_size > MAX_BUFFER_SIZE) {
        fprintf(stderr, "Error BufferAllocator: %s - Trying to allocated %lu which is more than the maximum of %lu\n", name, buffer_size, MAX_BUFFER_SIZE);
        abort();
    }

    printf("Buffer size: %ld\n", buffer_size);

    glNamedBufferStorage(_buffer, buffer_size, nullptr, GL_DYNAMIC_STORAGE_BIT);

    /* Fill free_blocks stack starting from last so index 0 ends at the top */
    for (int id = max_slots-1 ; id >= 0 ; --id) {
        _free_slots.push(id);
    }
}

BufferSlot BufferAllocator::allocate(uint32_t size, const void * data) {
    if (size == 0) {
        #ifdef PRINT_ERRORS
        fprintf(stderr, "Error: %s - Trying to allocate size of 0!\n", _name);
        #endif

        return invalid_buffer_slot;
    }

    if (size > _slot_size) {
        #ifdef PRINT_ERRORS
        fprintf(stderr, "Error: %s - Allocate size demanded %d is higher than maximum slot size of %ld\n", _name, size, _slot_size);
        exit(0);
        #endif

        return invalid_buffer_slot;
    }
    if (_free_slots.size() <= 0) {
        #ifdef PRINT_ERRORS
        fprintf(stderr, "Error: %s - No free slot in buffer\n", _name);
        #endif

        return invalid_buffer_slot;
    }

    uint32_t id = _free_slots.top();
    _free_slots.pop();

    #ifdef PRINT_ERRORS
    // printf("Info: %s - Allocating %d - ID %d == %ld\n", _name, size, id, id * _slot_size + size);
    #endif

    BufferSlot b = {
        .start = (int) (id * _slot_size),
        .size = (int) size,
        .id = (int) id
    };

    glNamedBufferSubData(
        _buffer,
        b.start,
        size,
        data
    );

    return b;
}

BufferSlot BufferAllocator::updateAllocation(uint32_t id, uint32_t size, const void *data) {
    if (size == 0) {
        #ifdef PRINT_ERRORS
        fprintf(stderr, "Error: %s - Trying to allocate size of 0!\n", _name);
        #endif
        return invalid_buffer_slot;
    }

    if (size > _slot_size) {
        #ifdef PRINT_ERRORS
        fprintf(stderr, "Error: %s - Allocate size demanded %d is higher than maximum slot size of %ld!\n", _name, size, _slot_size);
        #endif
        return invalid_buffer_slot;
    }

    BufferSlot b = {
        .start = (int) (id * _slot_size),
        .size = (int) size,
        .id = (int) id
    };

    glNamedBufferSubData(
        _buffer,
        b.start,
        size,
        data
    );

    return b;
}

void BufferAllocator::deallocate(int id) {
    if (id == -1) {
        #ifdef PRINT_ERRORS
        printf("Error: %s - Tried to deallocated indalid id %d!\n", _name, id);
        #endif
        return;
    }
    _free_slots.push(id);
}

GLuint BufferAllocator::getBufferObject() {
    return _buffer;
}
