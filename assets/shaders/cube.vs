#version 460 core

#extension GL_ARB_gpu_shader5 : enable
#extension GL_ARB_gpu_shader_int64 : enable

layout(binding = 1, std430) readonly buffer ssbo_chunk_positions {
    vec4 chunk_positions[];
};

layout(binding = 2, std430) readonly buffer ssbo_blocks_faces {
    uint64_t blocks_faces[];
};

out VS_OUT {
    vec3 frag_pos;
    vec2 uv;
    flat uint orientation;
    flat uint texture_id;
    float ambient_occlusion;
    vec4 FragPosLightSpace;
    flat uint isTranslucent;
} vs_out;

#include "uniforms.glsl"

const ivec2 model_face[8] = {
    ivec2(0, 0), ivec2(1, 0), ivec2(1, 1), ivec2(0, 1),
    ivec2(0, 0), ivec2(0, 1), ivec2(1, 1), ivec2(1, 0)
};

const ivec2 model_face_flipped[8] = {
    ivec2(1, 0), ivec2(1, 1), ivec2(0, 1), ivec2(0, 0),
    ivec2(0, 1), ivec2(1, 1), ivec2(1, 0), ivec2(0, 0)
};

const int ao_order[8] = {
    1, 2, 3, 0,
    3, 2, 1, 0
};

const int ao_order_flipped[8] = {
    0, 1, 2, 3,
    0, 3, 2, 1
};

ivec2 rotate_uv(ivec2 uv, int rot) {
    switch (rot) {
        case 0: return uv;
        case 1: return ivec2(uv.y, 1 - uv.x);     // 90°
        case 2: return ivec2(1 - uv.x, 1 - uv.y); // 180°
        case 3: return ivec2(1 - uv.y, uv.x);     // 270°
    }
    return uv;
}

void main() {
    uint64_t data = blocks_faces[gl_BaseInstance + gl_VertexID / 6];

    ivec3 block_pos = ivec3(
        int((data >> 0)  & 63),
        int((data >> 6)  & 63),
        int((data >> 12) & 63)
    );

    int orientation = int((data >> 18) & 7);
    int texture_id  = int((data >> 21) & 511);

    int ao[4] = {
        int((data >> 32) & 3),
        int((data >> 34) & 3),
        int((data >> 36) & 3),
        int((data >> 38) & 3)
    };

    uint isTranslucent = uint((data >> 40) & 1);

    int offset = (orientation == 1 || orientation == 3) ? 4 : 0;
    int vertex_index = (gl_VertexID % 4) + offset;

    bool flip = (ao[0] + ao[2]) > (ao[1] + ao[3]);
    const ivec2[] model = flip ? model_face : model_face_flipped;
    const int[] ao_index = flip ? ao_order_flipped : ao_order;

    ivec2 uv = model[vertex_index];
    vec3 model_offset;

    switch (orientation) {
        case 0: model_offset = vec3(uv.x, 1, uv.y); break; // TOP
        case 1: model_offset = vec3(uv.x, 0, uv.y); break; // BOTTOM
        case 2: model_offset = vec3(uv.x, uv.y, 0); break; // FRONT
        case 3: model_offset = vec3(uv.x, uv.y, 1); uv.x = 1 - uv.x; break; // BACK
        case 4: model_offset = vec3(0, uv.x, uv.y); uv = rotate_uv(uv, 3); break; // LEFT
        case 5: model_offset = vec3(1, uv.y, uv.x); break; // RIGHT
    }

    float ao_factor = float(ao[ao_index[vertex_index]]) / 3.0;

    // vec3 world_pos = chunk_positions[gl_DrawID].xyz + block_pos + model_offset;
    vec3 world_pos = chunk_positions[gl_DrawID].xyz + block_pos + model_offset;

    vs_out.FragPosLightSpace = uniforms.lightSpaceMatrix * vec4(world_pos, 1.0);
    vs_out.frag_pos = world_pos;
    vs_out.uv = uv;
    vs_out.orientation = orientation;
    vs_out.texture_id = texture_id;
    vs_out.ambient_occlusion = ao_factor;
    vs_out.isTranslucent = isTranslucent;

    gl_Position = uniforms.projection_view  * vec4(world_pos, 1.0);
}
