#version 460 core
#extension GL_ARB_bindless_texture : require

layout(binding = 0, std430) readonly buffer ssbo {
    sampler2D texture_handles[];
};

const vec3 orientation_normal_table[] = { // Note: maybe should be done in fragment shader to save bandwidth
    vec3(0.0, 1.0, 0.0), // Top = 0
    vec3(0.0, -1.0, 0.0), // Bottom = 1
    vec3(0.0, 0.0, -1.0), // Front = 2
    vec3(0.0, 0.0, 1.0), // Back = 3
    vec3(-1.0, 0.0, 0.0), // Left = 4
    vec3(1.0, 0.0, 0.0), // Right = 5
};

in vec3 f_frag_pos;
in vec2 f_uv;
flat in int f_orientation;
flat in int f_faceId;
in vec4 f_FragPosLightSpace;

out vec4 FragColor;

uniform vec3 u_sun_direction;
uniform vec3 u_view_position;

uniform sampler2D shadowMap;

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r;

    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    if (currentDepth > 1.0) {
        return 0.0;
    }
    currentDepth = (projCoords.z);

    // calculate bias (based on depth map resolution and slope)
    float cosTheta = dot(normal, u_sun_direction);
    float magic_bias_constant = 0.00035;
    float bias = magic_bias_constant*tan(acos(cosTheta));

    // check whether current frag pos is in shadow
    // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;

    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += (currentDepth - bias) > pcfDepth  ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if (projCoords.z > 1.0)
        shadow = 0.0;

    return shadow;
}

void main()
{
    vec4 color = texture(sampler2D(texture_handles[f_faceId]), f_uv).rgba;
    vec3 normal = orientation_normal_table[f_orientation];

    vec3 lightColor = vec3(255.0, 244.0, 196.0) / 255.0;

    // ambient
    float ambientStrength = 0.35;
    vec3 ambient = ambientStrength * lightColor;

    // diffuse
    float diff = max(dot(normal, u_sun_direction), 0.0);
    vec3 diffuse = diff * lightColor;

    // vec3 result = (ambient + diffuse) * color.rgb;

    if (color.a < 0.65) { // magic value
        discard;
    }

    // calculate shadow
    float shadow = ShadowCalculation(f_FragPosLightSpace, normal);

    if (dot(normal, u_sun_direction) < 0.0) // if cube face is not facing light, then it's in its own shadow
        shadow = 1.0;

    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse)) * color.rgb;
    // vec3 lighting = (ambient + (1.0 - shadow)) * color.rgb;

    FragColor = vec4(lighting, 1.0);
    // FragColor = vec4(normal, 1.0);
}
