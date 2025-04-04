#version 460 core

#extension GL_ARB_gpu_shader5 : enable
#extension GL_ARB_gpu_shader_int64 : enable

// layout (location = 0) in uint a_packedVertex;

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


const vec3 model_vertex[] = {
    // +Y
    vec3(0, 1, 0),
    vec3(1, 1, 0),
    vec3(1, 1, 1),

    vec3(0, 1, 0),
    vec3(1, 1, 1),
    vec3(0, 1, 1),

    // -Y
    vec3(0, 0, 0),
    vec3(1, 0, 1),
    vec3(1, 0, 0),

    vec3(0, 0, 0),
    vec3(0, 0, 1),
    vec3(1, 0, 1),


    // -Z
    vec3(0, 0, 0),
    vec3(1, 0, 0),
    vec3(1, 1, 0),

    vec3(0, 0, 0),
    vec3(1, 1, 0),
    vec3(0, 1, 0),


    // +Z
    vec3(0, 0, 1),
    vec3(1, 1, 1),
    vec3(1, 0, 1),

    vec3(0, 0, 1),
    vec3(0, 1, 1),
    vec3(1, 1, 1),



    // -X
    vec3(0, 0, 0),
    vec3(0, 1, 1),
    vec3(0, 0, 1),

    vec3(0, 0, 0),
    vec3(0, 1, 0),
    vec3(0, 1, 1),


    // +X
    vec3(1, 0, 0),
    vec3(1, 0, 1),
    vec3(1, 1, 1),

    vec3(1, 0, 0),
    vec3(1, 1, 1),
    vec3(1, 1, 0),
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

const vec3 model_vertex_flipped[] = {
    // +Y
    vec3(0, 1, 0),
    vec3(1, 1, 0),
    vec3(0, 1, 1),

    vec3(0, 1, 1),
    vec3(1, 1, 0),
    vec3(1, 1, 1),

    // -Y
    vec3(0, 0, 0),
    vec3(0, 0, 1),
    vec3(1, 0, 0),

    vec3(0, 0, 1),
    vec3(1, 0, 1),
    vec3(1, 0, 0),


    // -Z
    vec3(0, 0, 0),
    vec3(1, 0, 0),
    vec3(1, 1, 0),

    vec3(0, 0, 0),
    vec3(1, 1, 0),
    vec3(0, 1, 0),


    // +Z
    vec3(0, 0, 1),
    vec3(1, 1, 1),
    vec3(1, 0, 1),

    vec3(0, 0, 1),
    vec3(0, 1, 1),
    vec3(1, 1, 1),


    // -X
    vec3(0, 0, 0),
    vec3(0, 1, 1),
    vec3(0, 0, 1),

    vec3(0, 0, 0),
    vec3(0, 1, 0),
    vec3(0, 1, 1),


    // +X
    vec3(1, 0, 0),
    vec3(1, 0, 1),
    vec3(1, 1, 1),

    vec3(1, 0, 0),
    vec3(1, 1, 1),
    vec3(1, 1, 0),
};

const ivec2 model_uv_flipped[] = {
    // +Y
    ivec2(0, 0), ivec2(1, 0), ivec2(0, 1),
    ivec2(0, 1), ivec2(1, 0), ivec2(1, 1),

    // -Y
    ivec2(0, 0), ivec2(0, 1), ivec2(1, 0),
    ivec2(0, 1), ivec2(1, 1), ivec2(1, 0),

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

// if (face_vertex == 0)
//     a_ambient_occlusion = a_ambient_occlusion00;
// if (face_vertex == 1)
//     a_ambient_occlusion = a_ambient_occlusion11;
// if (face_vertex == 2)
//     a_ambient_occlusion = a_ambient_occlusion10;

// if (face_vertex == 3)
//     a_ambient_occlusion = a_ambient_occlusion00;
// if (face_vertex == 4)
//     a_ambient_occlusion = a_ambient_occlusion01;
// if (face_vertex == 5)
//     a_ambient_occlusion = a_ambient_occlusion11;

const int ao_order[] = {
    0, 1, 2,
    0, 2, 3
};

const int ao_order_flipped[] = {
    0, 1, 3,
    3, 1, 2
};

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

    int a_ambient_occlusion_4[4] = {
        a_ambient_occlusion00,
        a_ambient_occlusion10,
        a_ambient_occlusion11,
        a_ambient_occlusion01,
    };

    int a_ambient_occlusion = 3;

    int face_vertex = gl_VertexID % 6;

    if (a_orientation == 1 || a_orientation == 2 || a_orientation == 4) {
        if (face_vertex == 1) face_vertex = 2;
        else if (face_vertex == 2) face_vertex = 1;
        else if (face_vertex == 4) face_vertex = 5;
        else if (face_vertex == 5) face_vertex = 4;
    }

    if (a_orientation < 2) { // TOP
        a_ambient_occlusion = a_ambient_occlusion_4[ao_order[face_vertex]];
        // if (face_vertex == 0)
        //     a_ambient_occlusion = a_ambient_occlusion00;
        // if (face_vertex == 1)
        //     a_ambient_occlusion = a_ambient_occlusion10;
        // if (face_vertex == 2)
        //     a_ambient_occlusion = a_ambient_occlusion11;

        // if (face_vertex == 3)
        //     a_ambient_occlusion = a_ambient_occlusion00;
        // if (face_vertex == 4)
        //     a_ambient_occlusion = a_ambient_occlusion11;
        // if (face_vertex == 5)
        //     a_ambient_occlusion = a_ambient_occlusion01;
    }

    const int vertex_index = a_orientation*6 + gl_VertexID % 6;

    ivec3 a_position = ivec3(a_x, a_y, a_z);
    ivec2 a_uv = model_uv[vertex_index];
    vec3 model_offset = model_vertex[vertex_index];

    if (a_orientation < 2) // TOP
    if (a_ambient_occlusion00 + a_ambient_occlusion11 > a_ambient_occlusion01 + a_ambient_occlusion10) {
        // model_offset = model_vertex[vertex_index];
    } else {
        model_offset = model_vertex_flipped[vertex_index];
        a_uv = model_uv_flipped[vertex_index];

        a_ambient_occlusion = a_ambient_occlusion_4[ao_order_flipped[face_vertex]];

        // if (face_vertex == 0)
        //     a_ambient_occlusion = a_ambient_occlusion00;
        // if (face_vertex == 1)
        //     a_ambient_occlusion = a_ambient_occlusion10;
        // if (face_vertex == 2)
        //     a_ambient_occlusion = a_ambient_occlusion01;

        // if (face_vertex == 3)
        //     a_ambient_occlusion = a_ambient_occlusion01;
        // if (face_vertex == 4)
        //     a_ambient_occlusion = a_ambient_occlusion10;
        // if (face_vertex == 5)
        //     a_ambient_occlusion = a_ambient_occlusion11;
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
