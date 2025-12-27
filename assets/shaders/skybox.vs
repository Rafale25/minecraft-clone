#version 460 core

layout (location = 0) in vec3 aPos;

#include "uniforms.glsl"

uniform mat4 u_view;
uniform mat4 u_projection;

out vec3 fragPosLocal;
out vec3 fragPosWorld;

void main() {
    fragPosLocal = aPos;
    fragPosWorld = aPos + uniforms.viewPosition.xyz;
    gl_Position = u_projection * u_view * vec4(aPos, 1.0);
}
