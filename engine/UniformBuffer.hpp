#pragma once

#include <cassert>
#include <string>
#include <vector>
#include <unordered_map>
#include "Logger.hpp"
#include "VAO.hpp"
// #include "gl/gl.h"

struct UniformDefinition
{
    const char* name;
    int bytes;
};

struct UniformInfo
{
    int bytes;
    int offset;
};


typedef std::vector<UniformDefinition> UniformBufferDef;

inline static int getBytesPower(int n)
{
    const int N = 4;

    if (n <= N) return N;
    if (n <= N*2) return N*2;
    // if (n <= N*3) return N*3;
    if (n <= N*4) return N*4;
    if (n <= N*16) return N*16;
    return N*16;
}

class UniformBuffer {
public:
    // UniformBuffer() {}

    void makeBufferDef(const UniformBufferDef &def)
    {
        int size = 0;
        for (const auto &[name, bytes] : def) {
            if (_uniforms.contains(name)) {
                logE("Duplicate uniform {}", name);
                exit(-1);
            }
            _uniforms.insert({name, {bytes, size}});
            logD("added {:30} {:-2} -> {:-2} ; {:-3}", name, bytes, getBytesPower(bytes), size);

            size += getBytesPower(bytes);
        }

        logD("Buffer created with {} bytes", size);

        _buffer = createBufferStorage(nullptr, size);
    }

    void bind(int id)
    {
        glBindBufferBase(GL_UNIFORM_BUFFER, id, _buffer);
    }

    // void setValue(const char* name, const void *data) {
    template <typename T>
    void set(const char* name, const T& data) {
        const auto& it =_uniforms.find(name);
        if (it != _uniforms.end()) {
            if (sizeof(data) != it->second.bytes) {
                logE("{} data is {} instead of the {} registered", name, sizeof(data), it->second.bytes);
                exit(-1);
            }
            glNamedBufferSubData(_buffer, it->second.offset, it->second.bytes, &data);
        } else {
            logE("Uniform variable {} does not exist!", name);
        }
    }

// private:
public:
    std::unordered_map<std::string, UniformInfo> _uniforms;
    GLuint _buffer = 0;
};
