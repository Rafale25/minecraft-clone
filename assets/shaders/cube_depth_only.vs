#version 460 core

layout (location = 0) in uint a_packedVertex;

layout(binding = 1, std430) readonly buffer ssbo_chunk_positions {
    vec4 chunk_positions[];
};

layout(binding = 2, std430) readonly buffer ssbo_blocks_faces
{
    uint blocks_faces[];
};

out VS_OUT {
    out vec2 uv;
    flat uint texture_id;
} vs_out;

const vec3 model_vertex[] = {
    // +Y
    vec3(0.0, 1.0, 0.0),
    vec3(1.0, 1.0, 0.0),
    vec3(1.0, 1.0, 1.0),

    vec3(0.0, 1.0, 0.0),
    vec3(1.0, 1.0, 1.0),
    vec3(0.0, 1.0, 1.0),

    // -Y
    vec3(0.0, 0.0, 0.0),
    vec3(1.0, 0.0, 1.0),
    vec3(1.0, 0.0, 0.0),

    vec3(0.0, 0.0, 0.0),
    vec3(0.0, 0.0, 1.0),
    vec3(1.0, 0.0, 1.0),


    // -Z
    vec3(0.0, 0.0, 0.0),
    vec3(1.0, 0.0, 0.0),
    vec3(1.0, 1.0, 0.0),

    vec3(0.0, 0.0, 0.0),
    vec3(1.0, 1.0, 0.0),
    vec3(0.0, 1.0, 0.0),


    // +Z
    vec3(0.0, 0.0, 1.0),
    vec3(1.0, 1.0, 1.0),
    vec3(1.0, 0.0, 1.0),

    vec3(0.0, 0.0, 1.0),
    vec3(0.0, 1.0, 1.0),
    vec3(1.0, 1.0, 1.0),



    // -X
    vec3(0.0, 0.0, 0.0),
    vec3(0.0, 1.0, 1.0),
    vec3(0.0, 0.0, 1.0),

    vec3(0.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 1.0, 1.0),


    // +X
    vec3(1.0, 0.0, 0.0),
    vec3(1.0, 0.0, 1.0),
    vec3(1.0, 1.0, 1.0),

    vec3(1.0, 0.0, 0.0),
    vec3(1.0, 1.0, 1.0),
    vec3(1.0, 1.0, 0.0),
};

const ivec2 model_uv[] = {
    // +Y
    ivec2(0, 0), ivec2(1, 0), ivec2(1, 1),
    ivec2(0, 0), ivec2(1, 1), ivec2(0, 1),

    // -Y
    ivec2(0, 0), ivec2(1, 1), ivec2(1, 0),
    ivec2(0, 0), ivec2(0, 1), ivec2(1, 1),

    // -Z
    ivec2(0, 0), ivec2(1, 0), ivec2(1, 1),
    ivec2(0, 0), ivec2(1, 1), ivec2(0, 1),

    // +Z
    ivec2(0, 0), ivec2(1, 1), ivec2(1, 0),
    ivec2(0, 0), ivec2(0, 1), ivec2(1, 1),

    // -X
    ivec2(0, 0), ivec2(1, 1), ivec2(1, 0),
    ivec2(0, 0), ivec2(0, 1), ivec2(1, 1),

    // +X
    ivec2(0, 0), ivec2(1, 0), ivec2(1, 1),
    ivec2(0, 0), ivec2(1, 1), ivec2(0, 1),
};


uniform mat4 u_lightSpaceMatrix;

void main()
{
    const uint data = blocks_faces[gl_BaseInstance + gl_VertexID / 6];

    int a_x =                   int((data >> 0)  & 31);
    int a_y =                   int((data >> 5)  & 31);
    int a_z =                   int((data >> 10) & 31);
    uint a_orientation =           ((data >> 15) & 7);
    uint a_texture_id =            ((data >> 18) & 63);

    ivec3 a_position = ivec3(a_x, a_y, a_z);
    ivec2 a_uv = model_uv[a_orientation*6 + gl_VertexID % 6];
    vec3 model_offset = model_vertex[a_orientation*6 + gl_VertexID % 6];

    vec3 world_pos = chunk_positions[gl_DrawID].xyz + a_position + model_offset;
    vec4 position = u_lightSpaceMatrix * vec4(world_pos, 1.0);

    vs_out.uv = a_uv;
    vs_out.texture_id = a_texture_id;
    gl_Position = position;
}
