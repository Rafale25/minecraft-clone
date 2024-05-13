#version 460 core

in vec3 a_position;

uniform mat4 u_projectionMatrix;
uniform mat4 u_viewMatrix;

void main()
{
    vec4 position = u_projectionMatrix * u_viewMatrix * vec4(a_position, 1.0);
    gl_Position = position;
}
