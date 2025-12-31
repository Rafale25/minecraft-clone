#pragma once

#include "Logger.hpp"
#include "VAO.hpp"
#include <cassert>
#include <string>
#include <vector>
#include <unordered_map>

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
            // logD("added {:30} {:-2} -> {:-2} ; {:-3}", name, bytes, getBytesPower(bytes), size);

            size += getBytesPower(bytes);
        }

        // logD("Buffer created with {} bytes", size);

        _buffer = createBufferStorage(nullptr, size);
    }

    void bind(int id)
    {
        glBindBufferBase(GL_UNIFORM_BUFFER, id, _buffer);
    }

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


/* Example

_ubuffer.makeBufferDef({
    {"projection",                    sizeof(float)*16},
    {"view",                          sizeof(float)*16},
    {"projection_view",               sizeof(float)*16},
    {"lightSpaceMatrix",              sizeof(float)*16},
    {"sunDirection",                  sizeof(float)*4},
    {"viewPosition",                  sizeof(float)*4},
    {"resolution",                    sizeof(float)*2},
    {"sunDotAngle",                   sizeof(float)*1},
    {"FOV",                           sizeof(float)*1},
    {"fogDensity",                    sizeof(float)*1},
    {"shadow_bias",                   sizeof(float)*1},
    {"ambient_occlusion_strength",    sizeof(float)*1},
    {"time",                          sizeof(float)*1},
    {"exposure",                      sizeof(float)*1},
    {"ambient_occlusion_enabled",     sizeof(int)*1},
    {"tonemapping_enabled",           sizeof(int)*1},
});

_ubuffer.bind(0);

_ubuffer.set("projection", camera.getProjection());



// layout(std140, binding = 0) uniform uniformBuffer {
//     mat4 projection;
//     mat4 view;
//     mat4 projection_view;
//     mat4 lightSpaceMatrix;
//     vec4 sunDirection;
//     vec4 viewPosition;
//     vec2 resolution;
//     float sunDotAngle;
//     float FOV;
//     float fogDensity;
//     float shadow_bias;
//     float ambient_occlusion_strength;
//     float time;
//     float exposure;
//     int ambient_occlusion_enabled;
//     int tonemapping_enabled;
// } uniforms;



// DEBUG strides
for (const auto &[name, info] : _ubuffer._uniforms) {
    auto ix = glGetProgramResourceIndex(cube_shader.ID, GL_UNIFORM, (std::string("uniformBuffer.") + name).c_str());
    GLenum props[] = {GL_ARRAY_STRIDE, GL_OFFSET};
    GLint values[2] = {};
    glGetProgramResourceiv(cube_shader.ID, GL_UNIFORM, ix, 2, props, 2, NULL, values);

    logD("{}: {} {}", name, values[0], values[1]);
    auto byteOffset = values[1] + (3 * values[0]);
}
*/
