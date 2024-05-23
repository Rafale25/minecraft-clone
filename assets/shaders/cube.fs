#version 460 core
#extension GL_ARB_bindless_texture : require

layout(binding = 0, std430) readonly buffer ssbo {
    sampler2D texture_handles[];
};

in vec2 f_uv;
in int f_orientation;
flat in int f_faceId;

out vec4 FragColor;

const vec3 orientation_normal_table[] = { // Note: maybe should be done in fragment shader to save bandwidth
    vec3(0.0, 1.0, 0.0), // Top = 0
    vec3(0.0, -1.0, 0.0), // Bottom = 1
    vec3(0.0, 0.0, 1.0), // Front = 2
    vec3(0.0, 0.0, -1.0), // Back = 3
    vec3(-1.0, 0.0, 0.0), // Left = 4
    vec3(1.0, 0.0, 0.0), // Right = 5
};

void main()
{
    vec3 color = texture(sampler2D(texture_handles[f_faceId]), f_uv).rgb;
    vec3 normal = orientation_normal_table[f_orientation];

    FragColor = vec4(color, 1.0);
    // FragColor = vec4(f_uv, 0.0, 1.0);
}
