#include "BufferAllocator.hpp"

#include <glad/gl.h>

#include <stdio.h>
#include <stdlib.h>

BufferAllocator::BufferAllocator(const char* name, uint32_t slot_size, uint32_t max_slots): _name(name), _slot_size(slot_size), _max_slots(max_slots) {
    printf("Slotsize: %d\n", slot_size);

    glCreateBuffers(1, &_buffer);
    const uint buffer_size = slot_size * max_slots;

    printf("Buffer size: %d\n", buffer_size);

    glNamedBufferStorage(_buffer, buffer_size, nullptr, GL_DYNAMIC_STORAGE_BIT);

    /* Fill free_blocks stack starting from last so index 0 ends at the top */
    for (int id = max_slots-1 ; id >= 0 ; --id) {
        _free_slots.push(id);
    }
}

BufferSlot BufferAllocator::allocate(uint32_t size, const void * data) {
    if (size == 0) {
        // printf("Error: %s - Trying to allocate size of 0!\n", _name);
        return invalid_buffer_slot;
    }

    if (size > _slot_size) {
        printf("Error: %s - Allocate size demanded %d is higher than maximum slot size of %d\n", _name, size, _slot_size);
        return invalid_buffer_slot;
    }
    if (_free_slots.size() <= 0) {
        // printf("Error: %s - No free slot in buffer\n", _name);
        return invalid_buffer_slot;
    }

    // printf("Info: %s - Allocating %d\n", _name, size);

    uint32_t id = _free_slots.top();
    _free_slots.pop();

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
        // printf("Error: %s - Trying to allocate size of 0!\n", _name);
        return invalid_buffer_slot;
    }

    if (size > _slot_size) {
        // printf("Error: %s - Allocate size demanded %d is higher than maximum slot size of %d!\n", _name, size, _slot_size);
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
        printf("Error: %s - Tried to deallocated indalid id %d!\n", _name, id);
        return;
    }
    _free_slots.push(id);
}

GLuint BufferAllocator::getBufferObject() {
    return _buffer;
}
