#version 460 core

in vec3 a_position;

uniform mat4 u_projectionMatrix;
uniform mat4 u_viewMatrix;
uniform mat4 u_modelMatrix;

out vec3 v_worldPosition;

void main()
{
    vec4 worldPosition = u_modelMatrix * vec4(a_position, 1.0);
    vec4 position = u_projectionMatrix * u_viewMatrix * worldPosition;
    gl_Position = position;
    v_worldPosition = worldPosition.xyz;
}
