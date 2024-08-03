#pragma once

#include <cstdint>
#include <stddef.h>
#include <cstring>
#include <endian.h>

// TODO: add option to put bytes into buffer

class ByteBuffer
{
public:
    enum ByteOrder {
        LE,
        BE,
    };

public:
    // ByteBuffer(size_t size): size(size) {}
    ByteBuffer(uint8_t* buffer, size_t size, ByteOrder byte_order): buffer(buffer), head(buffer), size(size), byte_order(byte_order) {}
	~ByteBuffer() = default;

    uint8_t peek() {
        return *head;
    }

    uint8_t get() {
        uint8_t value = head[0];
        head += sizeof(uint8_t);
        return value;
    }

    void getN(uint8_t* p, size_t n) {
        memcpy(p, head, n);
        head += n * sizeof(uint8_t);
    }

    uint16_t getShort() {
        uint16_t value;
        memcpy(&value, head, sizeof(uint16_t));
        head += sizeof(uint16_t);

        if (byte_order == BE) return be16toh(value);
        return le16toh(value);
    }

    uint32_t getInt() {
        uint32_t value;
        memcpy(&value, head, sizeof(uint32_t));
        head += sizeof(uint32_t);

        if (byte_order == BE) return be32toh(value);
        return le32toh(value);
    }

    uint64_t getLong() {
        uint64_t value;
        memcpy(&value, head, sizeof(uint64_t));
        head += sizeof(uint64_t);

        if (byte_order == BE) return be64toh(value);
        return le64toh(value);
    }

    float getFloat() {
        // read float as if it was uint32_t
        uint32_t value;
        memcpy(&value, head, sizeof(uint32_t));

        // convert to different endianess
        if (byte_order == BE) value = be32toh(value);
        else value = le32toh(value);

        // move bytes into float variable without casting
        float float_value;
        memcpy(&float_value, &value, sizeof(uint32_t));

        head += sizeof(uint32_t);

        return float_value;
    }

private:
    uint8_t* buffer;
    uint8_t* head;
    size_t size;
    ByteOrder byte_order;
};
