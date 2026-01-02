#version 460 core

in vec3 a_position;

out vec3 f_worldPosition;

uniform mat4 u_modelMatrix;

#include "uniforms.glsl"

void main()
{
    vec4 worldPosition = u_modelMatrix * vec4(a_position, 1.0);
    vec4 position = uniforms.projection_view * worldPosition;
    gl_Position = position;
    f_worldPosition = worldPosition.xyz;
}
