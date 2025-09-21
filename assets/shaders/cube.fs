#version 460 core
#extension GL_ARB_bindless_texture : require

float rand(vec2 co){ return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453); }

layout(std430, binding = 0) readonly buffer ssbo_texture_handles {
    sampler2D texture_handles[];
};

const vec3 orientation_normal_table[] = {
    vec3(0.0, 1.0, 0.0), // Top = 0
    vec3(0.0, -1.0, 0.0), // Bottom = 1
    vec3(0.0, 0.0, -1.0), // Front = 2
    vec3(0.0, 0.0, 1.0), // Back = 3
    vec3(-1.0, 0.0, 0.0), // Left = 4
    vec3(1.0, 0.0, 0.0), // Right = 5
};

in VS_OUT {
    vec3 frag_pos;
    vec2 uv;
    flat uint orientation;
    flat uint texture_id;
    float ambient_occlusion;
    vec4 FragPosLightSpace;
    flat uint isTranslucent;
} fs_in;

#include "uniforms.glsl"

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec3 gPosition;

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

    // calculate bias (based on depth map resolution and slope)
    float cosTheta = dot(normal, normalize(uniforms.sunDirection.xyz));
    float magic_bias_constant = uniforms.shadow_bias;// 0.00035;
    float bias = magic_bias_constant*tan(acos(cosTheta));

    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y)
        {
            vec2 offset = vec2(x, y) + rand(projCoords.xy + vec2(x, y)); // smooth out shadows by using random offsets
            float pcfDepth = texture(shadowMap, projCoords.xy + offset * texelSize).r;
            shadow += (currentDepth - bias) > pcfDepth  ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if (projCoords.z > 1.0)
        shadow = 0.0;

    return shadow;
}

#include "utils/tonemapping.glsl"
#include "utils/SRGB.glsl"

void main()
{
    vec2 uv = (gl_FragCoord.xy - 0.5*uniforms.resolution.xy) / uniforms.resolution.y;

    // vec4 color = vec4(0.2, 1.0, 0.0, 1.0);
    vec4 color = texture(texture_handles[fs_in.texture_id], fs_in.uv).rgba;
    color.rgb = toLinearSRGB(color.rgb);// pow(color.rgb, vec3(2.2));

    vec3 normal = orientation_normal_table[fs_in.orientation];
    vec3 lightColor = vec3(255.0, 244.0, 196.0) / 255.0;

    // ambient
    float ambientStrength = 0.15;// 35;
    vec3 ambient = ambientStrength * lightColor;

    // diffuse
    float diff = max(dot(normal, normalize(uniforms.sunDirection.xyz)), 0.0);
    vec3 diffuse = diff * lightColor;

    if (fs_in.isTranslucent == 0 && color.a < 0.65) { // magic value
        discard;
    }

    // calculate shadow
    float shadow = ShadowCalculation(fs_in.FragPosLightSpace, normal);

    // if cube face is not facing light, then it's in its own shadow
    if (dot(normal, uniforms.sunDirection.xyz) < 0.0
    || uniforms.sunDirection.y < 0.0) // if sun is under the ground (points up)
        shadow = 1.0;

    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse)) * color.rgb;

    if (uniforms.ambient_occlusion_enabled == 1) {
        lighting = mix(lighting * (1.0 - uniforms.ambient_occlusion_strength), lighting, fs_in.ambient_occlusion);
    }

    // FragColor = vec4(lighting, color.a);
    gPosition = fs_in.frag_pos;
    // FragColor = vec4(fs_in.frag_pos, 1.0);

    if (uniforms.tonemapping_enabled == 1) {
        lighting = lottes(lighting.rgb * uniforms.exposure);
    }

    vec3 gammaCorrected = fromLinearToSRGB(lighting);// pow(lighting, vec3(1.0/2.2));
    FragColor = vec4(gammaCorrected, color.a);

    // FragColor = vec4(normal, 1.0);
}
