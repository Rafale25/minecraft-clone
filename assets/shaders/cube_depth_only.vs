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

const ivec2 model_face[8] = {
    ivec2(0, 0), ivec2(1, 0), ivec2(1, 1), ivec2(0, 1),
    ivec2(0, 0), ivec2(0, 1), ivec2(1, 1), ivec2(1, 0)
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

    int offset = (orientation == 1 || orientation == 3) ? 4 : 0;
    int vertex_index = (gl_VertexID % 4) + offset;

    ivec2 uv = model_face[vertex_index];
    vec3 model_offset;

    switch (orientation) {
        case 0: model_offset = vec3(uv.x, 1, uv.y); break; // TOP
        case 1: model_offset = vec3(uv.x, 0, uv.y); break; // BOTTOM
        case 2: model_offset = vec3(uv.x, uv.y, 0); break; // FRONT
        case 3: model_offset = vec3(uv.x, uv.y, 1); uv.x = 1 - uv.x; break; // BACK
        case 4: model_offset = vec3(0, uv.x, uv.y); uv = rotate_uv(uv, 3); break; // LEFT
        case 5: model_offset = vec3(1, uv.y, uv.x); break; // RIGHT
    }

    vec3 world_pos = chunk_positions[gl_DrawID].xyz + block_pos + model_offset;
    vec4 position = u_lightSpaceMatrix * vec4(world_pos, 1.0);

    vs_out.uv = uv;
    vs_out.texture_id = texture_id;
    gl_Position = position;
}
