#version 460 core

in vec2 TexCoords;

out vec4 FragColor;

layout (location = 0) uniform sampler2D colorTexture;
layout (location = 1) uniform sampler2D worldPosTexture;
layout (location = 2) uniform sampler2D depthTexture;

#include "uniforms.glsl"
#include "skyColor.glsl"

#include "utils/fog.glsl"

void main()
{
    vec2 uv = (gl_FragCoord.xy - 0.5*uniforms.resolution.xy) / uniforms.resolution.y;

    vec3 color = texture(colorTexture, TexCoords).rgb;
    // float depth = texture(depthTexture, TexCoords).r;
    // vec3 worldPos = texture(worldPosTexture, TexCoords).rgb;

    vec4 finalColor = vec4(color, 1.0);

    // if (depth != 1.0) // is not skybox
    {
        float night_factor = 2.0;
        float steepness = 8.0;
        float ambiant_factor = 1.0 - pow(2.0, -steepness*uniforms.sunDotAngle - night_factor);
        ambiant_factor = clamp(ambiant_factor, 0.25, 1.0);

        finalColor = vec4(color * ambiant_factor, 1.0);
    }

    FragColor = finalColor;
}
