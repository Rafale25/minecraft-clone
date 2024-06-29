#version 460 core
#extension GL_ARB_bindless_texture : require

layout (location = 0) in vec3 a_position;

uniform mat4 u_lightSpaceMatrix;
uniform vec3 u_chunkPos;

void main()
{
    vec3 world_pos = u_chunkPos + a_position;
    vec4 position = u_lightSpaceMatrix * vec4(world_pos, 1.0);

    gl_Position = position;
}
