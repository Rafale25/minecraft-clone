#version 460 core
// #extension GL_ARB_bindless_texture : require

// layout(binding = 0, std430) readonly buffer ssbo {
//     sampler2D texture_handles[];
// };

// out vec3 frag_pos;
// in vec2 f_uv;
// flat in int f_faceId;

// out vec4 FragColor;

void main()
{
    // vec4 color = texture(sampler2D(texture_handles[f_faceId]), f_uv).rgba;

    // if (color.a < 0.65) { // magic value
    //     discard;
    // }

    // FragColor = vec4(result, 1.0);
}
