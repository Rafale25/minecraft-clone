#version 460 core
#extension GL_ARB_bindless_texture : require

layout(binding = 0, std430) readonly buffer ssbo {
    sampler2D texture_handles[];
};

in VS_OUT {
    in vec2 uv;
    flat in uint texture_id;
} fs_in;

void main()
{
    vec4 color = texture(sampler2D(texture_handles[fs_in.texture_id]), fs_in.uv).rgba;

    if (color.a < 0.65) { // magic value
        discard;
    }
}
