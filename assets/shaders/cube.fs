#version 460 core
#extension GL_ARB_bindless_texture : require

layout(binding = 0, std430) readonly buffer ssbo {
    sampler2D texture_handles[];
};

const vec3 orientation_normal_table[] = { // Note: maybe should be done in fragment shader to save bandwidth
    vec3(0.0, 1.0, 0.0), // Top = 0
    vec3(0.0, -1.0, 0.0), // Bottom = 1
    vec3(0.0, 0.0, 1.0), // Front = 2
    vec3(0.0, 0.0, -1.0), // Back = 3
    vec3(-1.0, 0.0, 0.0), // Left = 4
    vec3(1.0, 0.0, 0.0), // Right = 5
};

out vec3 frag_pos;
in vec2 f_uv;
flat in int f_orientation;
flat in int f_faceId;

out vec4 FragColor;

uniform vec3 u_sun_direction = normalize(vec3(0.2, -0.9, 0.5));
uniform vec3 u_view_position;

void main()
{
    vec4 color = texture(sampler2D(texture_handles[f_faceId]), f_uv).rgba;
    vec3 normal = orientation_normal_table[f_orientation];

    vec3 lightColor = vec3(255.0, 244.0, 196.0) / 255.0;

    // ambient
    float ambientStrength = 0.35;
    vec3 ambient = ambientStrength * lightColor;

    // diffuse
    vec3 norm = normalize(normal);
    vec3 lightDir = -u_sun_direction;
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    vec3 result = (ambient + diffuse) * color.rgb;

    if (color.a < 0.65) { // magic value
        discard;
    }

    FragColor = vec4(result, 1.0);
}
