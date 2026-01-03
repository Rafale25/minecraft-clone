#define MAT4 mat4
#define VEC4 vec4
#define VEC2 vec2
#define INIT(x)

layout(std430, binding=3) readonly buffer uniformsBuffer {
#include "../../src/game/renderer/uniforms_raw"
} uniforms;
