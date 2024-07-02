#version 460 core
#extension GL_ARB_bindless_texture : require

layout (location = 0) in ivec3 a_packedVertex;

uniform mat4 u_lightSpaceMatrix;
uniform vec3 u_chunkPos;

void main()
{
    int a_x =           int((a_packedVertex >> 0)  & 31);
    int a_y =           int((a_packedVertex >> 5)  & 31);
    int a_z =           int((a_packedVertex >> 10) & 31);
    // int a_u =           int((a_packedVertex >> 15) & 1);
    // int a_v =           int((a_packedVertex >> 16) & 1);
    // int a_orientation = int((a_packedVertex >> 17) & 7);
    ivec3 a_position = ivec3(a_x, a_y, a_z);

    vec3 world_pos = u_chunkPos + a_position;
    vec4 position = u_lightSpaceMatrix * vec4(world_pos, 1.0);

    gl_Position = position;
}
