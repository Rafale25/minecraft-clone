#pragma once

#include <glad/gl.h>

struct FormatNode {
    int size; // 1, 2, 4, 8
    int count;
    int type; // GL_UNSIGNED_BYTE, GL_HALF_FLOAT, GL_FLOAT, GL_DOUBLE
    bool normalize;
    bool valid;

    static FormatNode invalid() {
        return {
            .size = 0,
            .count = 0,
            .type = 0,
            .normalize = false,
            .valid = false
        };
    }
};

struct FormatInfo {
    int size;
    int nodes;
    int divisor;
    bool valid;

    static FormatInfo invalid() {
        return {
            .size = 0,
            .nodes = 0,
            .divisor = 0,
            .valid = false
        };
    }
};

struct FormatIterator {
    const char* ptr;
    FormatNode node;

    FormatIterator(const char* str): ptr(str) {}

    FormatInfo info();
    FormatNode next();
};

GLuint createBuffer(const void* data, GLsizeiptr size, GLenum usage = GL_DYNAMIC_STORAGE_BIT);
GLuint createVAO(GLuint buffer, const char * const format, GLuint element_buffer_object=GL_INVALID_INDEX);
