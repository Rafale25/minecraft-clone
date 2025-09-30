#version 330 core

layout (location = 0) in vec3 aPos;

uniform mat4 u_view;
uniform mat4 u_projection;

out vec3 fragPos;

void main() {
    fragPos = aPos;
    gl_Position = u_projection * u_view * vec4(aPos * 100.0, 1.0); // multiply by 100.0 to avoid clipping with high FOV (>120)
}
