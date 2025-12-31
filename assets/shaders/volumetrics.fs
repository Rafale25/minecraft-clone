#version 460 core

in vec2 TexCoords;

out vec4 FragColor;

// layout (location = 0) uniform sampler2D colorTexture;
layout (location = 1) uniform sampler2D worldPosTexture;
// layout (location = 2) uniform sampler2D depthTexture;
layout (location = 3) uniform sampler2DArray u_shadowmap;

#include "uniforms.glsl"
#include "skyColor.glsl"

#include "utils/fog.glsl"
#include "utils/tonemapping.glsl"
#include "utils/SRGB.glsl"
#include "raymarching.glsl"

uniform float test_slider_0 = 0.0;
uniform float test_slider_1 = 0.0;

void main()
{
    vec2 res = uniforms.resolution / 4.0;
    vec2 uv = (gl_FragCoord.xy - 0.5*res.xy) / res.y;

    vec3 worldPos = texture(worldPosTexture, TexCoords).rgb;
    float dist = distance(uniforms.viewPosition.xyz, worldPos);

    float volumetric_light = raymarchVolumetricLighting(worldPos, uniforms.volumetricDensity, uniforms.volumetricHGphasePower);
    // bool is_in_shadow = isInShadow(u_shadowmap, uniforms.view, worldPos);
    // if (volumetric_light > 0.0) {
    //     finalColor = vec3(1.0, 0.0, 0.0);
    // }

    const vec3 sunColor = vec3(1.0, 0.9, 0.7);
    vec3 finalColor = sunColor * volumetric_light;

    FragColor = vec4(finalColor, 1.0);
}
