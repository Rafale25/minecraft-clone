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
    out vec3 frag_pos;
    out vec2 uv;
    flat out uint orientation;
    flat uint texture_id;
    float ambient_occlusion;
    out vec4 FragPosLightSpace;
} vs_out;

uniform mat4 u_projection_view;
uniform mat4 u_lightSpaceMatrix;

const ivec2 model_face[] = {
    ivec2(0, 0),
    ivec2(1, 0),
    ivec2(1, 1),
    ivec2(0, 1),

    ivec2(0, 0),
    ivec2(0, 1),
    ivec2(1, 1),
    ivec2(1, 0),
};

const ivec2 model_face_flipped[] = {
    ivec2(1, 0),
    ivec2(1, 1),
    ivec2(0, 1),
    ivec2(0, 0),

    ivec2(0, 1),
    ivec2(1, 1),
    ivec2(1, 0),
    ivec2(0, 0),
};

const int ao_order[] = {
    1, 2, 3, 0,
    3, 2, 1, 0
};

const int ao_order_flipped[] = {
    0, 1, 2, 3,
    0, 3, 2, 1
};

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

    int a_x =                   int((data >> 0)  & 31);
    int a_y =                   int((data >> 5)  & 31);
    int a_z =                   int((data >> 10) & 31);
    int a_orientation =         int((data >> 15) & 7);
    int a_texture_id =          int((data >> 18) & 255);
    int a_ambient_occlusion00 = int((data >> 32) & 7);
    int a_ambient_occlusion10 = int((data >> 35) & 7);
    int a_ambient_occlusion11 = int((data >> 38) & 7);
    int a_ambient_occlusion01 = int((data >> 41) & 7);

    ivec3 a_position = ivec3(a_x, a_y, a_z);

    const int a_ambient_occlusion_4[4] = {
        a_ambient_occlusion00,
        a_ambient_occlusion10,
        a_ambient_occlusion11,
        a_ambient_occlusion01,
    };

    int a_ambient_occlusion = 3;

    int offset = a_orientation == 1 || a_orientation == 3 ? 4 : 0;

    int vertex_index = gl_VertexID % 4 + offset;

    bool shouldFlipFace = a_ambient_occlusion00 + a_ambient_occlusion11 > a_ambient_occlusion01 + a_ambient_occlusion10;
    const ivec2 model[] = shouldFlipFace ? model_face : model_face_flipped;

    ivec2 a_uv = model[vertex_index];

    vec3 model_offset;
    if (a_orientation == 0) // TOP
        model_offset = vec3(model[vertex_index].x, 1, model[vertex_index].y);
    else if (a_orientation == 1) // BOTTOM
        model_offset = vec3(model[vertex_index].x, 0, model[vertex_index].y);
    else if (a_orientation == 2) // FRONT
        model_offset = vec3(model[vertex_index].x, model[vertex_index].y, 0);
    else if (a_orientation == 3) // BACK
        model_offset = vec3(model[vertex_index].x, model[vertex_index].y, 1);
    else if (a_orientation == 4) // LEFT
        model_offset = vec3(0, model[vertex_index].x, model[vertex_index].y);
    else if (a_orientation == 5) // RIGHT
        model_offset = vec3(1, model[vertex_index].y, model[vertex_index].x);


    a_ambient_occlusion = shouldFlipFace ? a_ambient_occlusion_4[ao_order_flipped[vertex_index]]
                                         : a_ambient_occlusion_4[ao_order[vertex_index]];

    if (a_orientation == 3) {
        a_uv.x = 1 - a_uv.x;
    }
    if (a_orientation == 4) {
        a_uv = rotate_uv(a_uv, 3);
    }

    vec3 world_pos = chunk_positions[gl_DrawID].xyz + a_position + model_offset;
    vec4 position = u_projection_view * vec4(world_pos, 1.0);

    vs_out.FragPosLightSpace = u_lightSpaceMatrix * vec4(world_pos, 1.0);
    vs_out.frag_pos = world_pos;
    vs_out.uv = a_uv;
    vs_out.orientation = (a_orientation);
    vs_out.texture_id = a_texture_id;
    vs_out.ambient_occlusion = float(a_ambient_occlusion) / 3.0;
    gl_Position = position;
}
