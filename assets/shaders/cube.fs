#version 460 core
#extension GL_ARB_bindless_texture : require

layout(binding = 0, std430) readonly buffer ssbo {
    sampler2D texture_handles[];
};

in vec2 f_uv;
flat in int f_faceId;

out vec4 FragColor;

void main()
{

    vec3 color = texture(sampler2D(texture_handles[f_faceId]), f_uv).rgb;

    FragColor = vec4(color, 1.0);
    // FragColor = vec4(f_uv, 0.0, 1.0);
}
