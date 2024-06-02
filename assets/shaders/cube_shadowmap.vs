#version 460 core
#extension GL_ARB_bindless_texture : require

in vec3 a_position;
// in vec2 a_uv;
// in float a_orientation;

uniform mat4 u_lightSpaceMatrix;
uniform vec3 u_chunkPos;

void main()
{
    vec3 world_pos = u_chunkPos + a_position;
    vec4 position = u_lightSpaceMatrix * vec4(world_pos, 1.0);

    gl_Position = position;
}
