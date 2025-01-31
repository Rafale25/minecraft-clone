#version 460 core

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_color;

uniform mat4 u_viewProjection;

out vec3 v_color;
out vec3 v_worldPosition;

void main()
{
    vec4 position = u_viewProjection * vec4(a_position, 1.0);
    v_color = a_color;
    v_worldPosition = a_position;
    gl_Position = position;
}
