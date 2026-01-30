#version 460 core

in vec2 TexCoords;

out vec4 FragColor;

layout (location = 0) uniform sampler2D colorTexture;
// layout (location = 1) uniform sampler2D worldPosTexture;
// layout (location = 2) uniform sampler2D depthTexture;

#include "uniforms.glsl"
#include "skyColor.glsl"

#include "utils/fog.glsl"
#include "utils/tonemapping.glsl"
#include "utils/SRGB.glsl"

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

        // finalColor = color * ambiant_factor;
    }

    if (uniforms.tonemapping_enabled == 1) {
        finalColor = lottes(finalColor * uniforms.exposure);
    }
    finalColor = fromLinearToSRGB(finalColor);// pow(lighting, vec3(1.0/2.2));

    FragColor = vec4(finalColor, 1.0);
}
