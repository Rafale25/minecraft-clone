#version 460 core
#extension GL_ARB_bindless_texture : require

layout (location = 0) in uint a_packedVertex;

out VS_OUT {
    out vec3 frag_pos;
    out vec2 uv;
    flat out uint orientation;
    flat uint texture_id;
    out vec4 FragPosLightSpace;
} vs_out;

uniform mat4 u_projectionMatrix;
uniform mat4 u_viewMatrix;
uniform vec3 u_chunkPos;
uniform mat4 u_lightSpaceMatrix;

void main()
{
    int a_x =           int((a_packedVertex >> 0)  & 31);
    int a_y =           int((a_packedVertex >> 5)  & 31);
    int a_z =           int((a_packedVertex >> 10) & 31);
    int a_u =           int((a_packedVertex >> 15) & 1);
    int a_v =           int((a_packedVertex >> 16) & 1);
    uint a_orientation =   ((a_packedVertex >> 17) & 7);
    uint a_texture_id =    ((a_packedVertex >> 20) & 255);

    ivec3 a_position = ivec3(a_x, a_y, a_z);
    ivec2 a_uv = ivec2(a_u, a_v);

    vec3 world_pos = u_chunkPos + a_position;
    vec4 position = u_projectionMatrix * u_viewMatrix * vec4(world_pos, 1.0);

    vs_out.FragPosLightSpace = u_lightSpaceMatrix * vec4(world_pos, 1.0);
    vs_out.frag_pos = world_pos;
    vs_out.uv = a_uv;
    vs_out.orientation = (a_orientation);
    vs_out.texture_id = a_texture_id;
    gl_Position = position;
}
