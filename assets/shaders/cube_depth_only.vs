#version 460 core

#extension GL_ARB_gpu_shader5 : enable
#extension GL_ARB_gpu_shader_int64 : enable

layout(binding = 1, std430) readonly buffer ssbo_chunk_positions {
    vec4 chunk_positions[];
};

layout(binding = 2, std430) readonly buffer ssbo_blocks_faces
{
    uint64_t blocks_faces[];
};

out VS_OUT {
    out vec2 uv;
    flat uint texture_id;
} vs_out;

const ivec2 model[] = {
    ivec2(0, 0),
    ivec2(1, 0),
    ivec2(1, 1),
    ivec2(0, 1),

    ivec2(0, 0),
    ivec2(0, 1),
    ivec2(1, 1),
    ivec2(1, 0),
};

uniform mat4 u_lightSpaceMatrix;

ivec2 rotate_uv(ivec2 uv, int rot) {
    if (rot == 0) return uv;
    if (rot == 1) return ivec2(uv.y, 1.0 - uv.x); // 90°
    if (rot == 2) return ivec2(1.0 - uv.x, 1.0 - uv.y); // 180°
    if (rot == 3) return ivec2(1.0 - uv.y, uv.x); // 270°
    return uv;
}

void main()
{
    const uint64_t data = blocks_faces[gl_BaseInstance + gl_VertexID / 6];

    ivec3 block_pos = ivec3(
        int((data >> 0)  & 63),
        int((data >> 6)  & 63),
        int((data >> 12) & 63)
    );
    int orientation = int((data >> 18) & 7);
    int texture_id  = int((data >> 21) & 511);


    int vertex_index = gl_VertexID % 4;// + offset;
    ivec2 a_uv = model[vertex_index];

    if (orientation == 3) {
        a_uv.x = 1 - a_uv.x;
    }
    if (orientation == 4) {
        a_uv = rotate_uv(a_uv, 3);
    }

    vec3 model_offset;
    if (orientation == 0) // TOP
        model_offset = vec3(model[vertex_index].x, 1, model[vertex_index].y);
    else if (orientation == 1) // BOTTOM
        model_offset = vec3(model[vertex_index].x, 0, model[vertex_index].y);
    else if (orientation == 2) // FRONT
        model_offset = vec3(model[vertex_index].x, model[vertex_index].y, 0);
    else if (orientation == 3) // BACK
        model_offset = vec3(model[vertex_index].x, model[vertex_index].y, 1);
    else if (orientation == 4) // LEFT
        model_offset = vec3(0, model[vertex_index].x, model[vertex_index].y);
    else if (orientation == 5) // RIGHT
        model_offset = vec3(1, model[vertex_index].y, model[vertex_index].x);


    vec3 world_pos = chunk_positions[gl_DrawID].xyz + block_pos + model_offset;
    vec4 position = u_lightSpaceMatrix * vec4(world_pos, 1.0);

    vs_out.uv = a_uv;
    vs_out.texture_id = texture_id;
    gl_Position = position;
}
