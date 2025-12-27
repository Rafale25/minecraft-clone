#version 460 core

in vec2 TexCoords;

out vec4 FragColor;

layout (location = 0) uniform sampler2D colorTexture;
layout (location = 1) uniform sampler2D worldPosTexture;
// layout (location = 2) uniform sampler2D depthTexture;
layout (location = 3) uniform sampler2DArray u_shadowmap;

#include "uniforms.glsl"
#include "skyColor.glsl"

#include "shadowmapping.glsl"
#include "utils/fog.glsl"
#include "utils/tonemapping.glsl"
#include "utils/SRGB.glsl"

uniform float test_slider_0 = 0.0;
uniform float test_slider_1 = 0.0;
uniform float test_slider_2 = 0.0;

float hgPhase(float cosTheta, float g)
{
    #define PI (3.14159265359)
    return (1.0 - g*g) / (4.0 * PI * pow(1.0 + g*g - 2.0*g*cosTheta, 1.5));
}

float raymarchVolumetricLighting(vec3 end)//, float depth)
{
    const float maxDistance = 100.0;

    vec3 startPos = uniforms.viewPosition.xyz;
    vec3 endPos = end;
    // if (depth == 1.0) {
    //     endPos = uniforms.viewPosition
    // }

    vec3 ray = endPos - startPos;
    vec3 rayDirection = normalize(ray);
    float rayLength = clamp(length(ray), 0, maxDistance);

    int steps = 32;
    float stepSize = rayLength / steps;
    vec3 stepVec = rayDirection * stepSize;

    vec3 currentPos = startPos + rayDirection * rand(gl_FragCoord.xy * 0.01) * (stepSize*2);

    float L = 0.0;
    float T = 1.0;

    // float accumulatedLight = 0.0;
    for (int i = 0 ; i < steps - 1; ++i) {
        const float density = test_slider_0;//0.05; //0.15;

        float stepTransmittance = exp(-density * stepSize);

        bool lit = !isInShadow(u_shadowmap, uniforms.view, currentPos);
        // accumulatedLight += float(lit);

        if (lit) {
            float cosTheta = dot(-uniforms.sunDirection.xyz, -rayDirection);
            // float phase = hgPhase(cosTheta, 0.6);
            float phase = hgPhase(cosTheta, test_slider_1);

            float scatteredLight = 1.0 * phase * density;

            // Only the light that survives so far contributes
            L += T * scatteredLight * (1.0 - stepTransmittance);
        }

        // T *= stepTransmittance;

        if (T < 0.01) break;

        currentPos += stepVec;
    }

    // accumulatedLight /= steps;

    return L;
}

uniform bool TRUE = true;

void main()
{
    vec2 uv = (gl_FragCoord.xy - 0.5*uniforms.resolution.xy) / uniforms.resolution.y;

    vec3 color = texture(colorTexture, TexCoords).rgb;

    vec3 worldPos = texture(worldPosTexture, TexCoords).rgb;
    // float depth = texture(depthTexture, TexCoords).r;

    float dist = distance(uniforms.viewPosition.xyz, worldPos);

    vec3 finalColor = color;

    // if (depth != 1.0) // is not skybox
    {
        float night_factor = 2.0;
        float steepness = 8.0;
        float ambiant_factor = 1.0 - pow(2.0, -steepness*uniforms.sunDotAngle - night_factor);
        ambiant_factor = clamp(ambiant_factor, 0.25, 1.0);

        finalColor = color * ambiant_factor;
    }


    float volumetric_light = raymarchVolumetricLighting(worldPos);
    // bool is_in_shadow = isInShadow(u_shadowmap, uniforms.view, worldPos);
    // if (volumetric_light > 0.0) {
    //     finalColor = vec3(1.0, 0.0, 0.0);
    // }

    // if (!TRUE)
    vec3 sunColor = vec3(1.0, 0.9, 0.7);
    finalColor += sunColor * volumetric_light * test_slider_2;
    // finalColor += vec3(volumetric_light) * 1.0;

    if (uniforms.tonemapping_enabled == 1) {
        finalColor = lottes(finalColor * uniforms.exposure);
    }
    finalColor = fromLinearToSRGB(finalColor);// pow(lighting, vec3(1.0/2.2));

    // if (TRUE)
    //     finalColor = worldPos.xyz;//mix(worldPos.xyz, finalColor.xyz, 0.01);
    //     finalColor = mix(vec3(volumetric_light), finalColor.xyz, 0.1);

    FragColor = vec4(finalColor, 1.0);
}
