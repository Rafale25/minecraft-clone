#version 460 core
#extension GL_ARB_bindless_texture : require

layout (location = 0) in int a_packedVertex;

layout(binding = 1, std430) readonly buffer ssbo_chunk_positions {
    vec4 chunk_positions[];
};

out VS_OUT {
    out vec2 uv;
    flat uint texture_id;
} vs_out;

uniform mat4 u_lightSpaceMatrix;
uniform vec3 u_chunkPos;

void main()
{
    int a_x =           int((a_packedVertex >> 0)  & 31);
    int a_y =           int((a_packedVertex >> 5)  & 31);
    int a_z =           int((a_packedVertex >> 10) & 31);
    int a_u =           int((a_packedVertex >> 15) & 1);
    int a_v =           int((a_packedVertex >> 16) & 1);
    // uint a_orientation =   ((a_packedVertex >> 17) & 7);
    uint a_texture_id =    ((a_packedVertex >> 20) & 255);

    ivec3 a_position = ivec3(a_x, a_y, a_z);

    ivec2 a_uv = ivec2(a_u, a_v);

    vec3 world_pos = chunk_positions[gl_DrawID].xyz + a_position;
    vec4 position = u_lightSpaceMatrix * vec4(world_pos, 1.0);

    vs_out.uv = a_uv;
    vs_out.texture_id = a_texture_id;
    gl_Position = position;
}
