#version 460 core

in vec2 TexCoords;

out vec4 FragColor;

layout (location = 0) uniform sampler2D colorTexture;
// layout (location = 1) uniform sampler2D worldPosTexture;
// layout (location = 2) uniform sampler2D depthTexture;
// layout (location = 3) uniform sampler2DArray u_shadowmap;

#include "uniforms.glsl"
#include "skyColor.glsl"

#include "utils/fog.glsl"
#include "utils/tonemapping.glsl"
#include "utils/SRGB.glsl"
// #include "raymarching.glsl"

void main()
{
    vec2 uv = (gl_FragCoord.xy - 0.5*uniforms.resolution.xy) / uniforms.resolution.y;

    vec3 color = texture(colorTexture, TexCoords).rgb;

    // vec3 worldPos = texture(worldPosTexture, TexCoords).rgb;
    // float depth = texture(depthTexture, TexCoords).r;

    // float dist = distance(uniforms.viewPosition.xyz, worldPos);

    vec3 finalColor = color;

    // if (depth != 1.0) // is not skybox
    {
        float night_factor = 2.0;
        float steepness = 8.0;
        float ambiant_factor = 1.0 - pow(2.0, -steepness*uniforms.sunDotAngle - night_factor);
        ambiant_factor = clamp(ambiant_factor, 0.25, 1.0);

        finalColor = color * ambiant_factor;
    }


    // float volumetric_light = raymarchVolumetricLighting(worldPos, test_slider_0, test_slider_1);
    // bool is_in_shadow = isInShadow(u_shadowmap, uniforms.view, worldPos);
    // if (volumetric_light > 0.0) {
    //     finalColor = vec3(1.0, 0.0, 0.0);
    // }

    // vec3 sunColor = vec3(1.0, 0.9, 0.7);
    // finalColor += sunColor * volumetric_light;

    if (uniforms.tonemapping_enabled == 1) {
        finalColor = lottes(finalColor * uniforms.exposure);
    }
    finalColor = fromLinearToSRGB(finalColor);// pow(lighting, vec3(1.0/2.2));

    // if (TRUE)
    //     finalColor = worldPos.xyz;//mix(worldPos.xyz, finalColor.xyz, 0.01);
    //     finalColor = mix(vec3(volumetric_light), finalColor.xyz, 0.1);

    FragColor = vec4(finalColor, 1.0);
}
