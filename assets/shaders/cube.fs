#version 460 core
#extension GL_ARB_bindless_texture : require

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
#include "shadowmapping.glsl"
#include "utils/tonemapping.glsl"
#include "utils/SRGB.glsl"
#include "utils/fog.glsl"
#include "skyColor.glsl"

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec3 gPosition;

uniform sampler2DArray u_shadowmap;

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
    float shadow = ShadowCalculation(u_shadowmap, uniforms.view, fs_in.frag_pos, normal, uniforms.viewPosition.xyz, normalize(uniforms.sunDirection.xyz), uniforms.shadow_bias);

    // if cube face is not facing light, then it's in its own shadow
    if (dot(normal, uniforms.sunDirection.xyz) < 0.0
    || uniforms.sunDirection.y < 0.0) // if sun is under the ground (points up)
        shadow = 1.0;

    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse)) * color.rgb;

    if (uniforms.ambient_occlusion_enabled == 1) {
        lighting = mix(lighting * (1.0 - uniforms.ambient_occlusion_strength), lighting, fs_in.ambient_occlusion);
    }

    gPosition = fs_in.frag_pos;

    if (uniforms.tonemapping_enabled == 1) {
        lighting = lottes(lighting.rgb * uniforms.exposure);
    }
    lighting = fromLinearToSRGB(lighting);// pow(lighting, vec3(1.0/2.2));

    { // FOG
        vec3 worldPos = fs_in.frag_pos;
        vec3 delta = worldPos - uniforms.viewPosition.xyz;
        float fragDistance = length(delta);
        vec3 ray = normalize(delta);
        vec3 skyColor = getSkyColor(ray, uniforms.sunDotAngle);
        vec3 rd = normalize(worldPos - uniforms.viewPosition.xyz);
        lighting = applyFog(lighting, fragDistance, rd, uniforms.sunDirection.xyz, skyColor, uniforms.fogDensity);
    }

// #define DEBUG_SHADOWMAP_LAYER
#ifdef DEBUG_SHADOWMAP_LAYER
    int layer = getShadowMapLayer(uniforms.view, fs_in.frag_pos);
    vec3 layerColor;
    if (layer == 0) {
        layerColor = vec3(1.0, 0.0, 0.0);
    } else if (layer == 1) {
        layerColor = vec3(0.0, 1.0, 0.0);
    } else if (layer == 2) {
        layerColor = vec3(0.0, 0.0, 1.0);
    } else if (layer == 3) {
        layerColor = vec3(1.0, 0.0, 1.0);
    }

    lighting = mix(lighting, layerColor, 0.4);
#endif

    FragColor = vec4(lighting, color.a);
    // FragColor = vec4(normal, 1.0);
    // FragColor = vec4(fs_in.frag_pos, 1.0);
}
