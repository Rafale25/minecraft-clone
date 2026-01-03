#version 460 core

layout (location = 0) in vec3 aPos;

#include "uniforms.glsl"

out vec3 fragPosLocal;
out vec3 fragPosWorld;

void main() {
    fragPosLocal = aPos;
    fragPosWorld = aPos + uniforms.viewPosition.xyz;
    gl_Position = uniforms.projection_view_noviewtranslate * vec4(aPos, 1.0);
}
