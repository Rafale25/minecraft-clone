#version 460 core

out vec4 FragColor;

in vec2 TexCoords;

layout (location = 0) uniform sampler2D u_scene;
layout (location = 1) uniform sampler2D u_bloomBlur;

#include "uniforms.glsl"

// vec3 bloom_none()
// {
//     vec3 hdrColor = texture(u_scene, texCoord).rgb;
//     return hdrColor;
// }

vec3 upsample(sampler2D samplerTexture, vec2 texcoords, float aspectRatio, float filterRadius=0.00085)
{
    // The filter kernel is applied with a radius, specified in texture
    // coordinates, so that the radius will vary across mip resolutions.
    float x = filterRadius;
    float y = filterRadius * aspectRatio;

    // Take 9 samples around current texel:
    // a - b - c
    // d - e - f
    // g - h - i
    // === ('e' is the current texel) ===
    vec3 a = texture(samplerTexture, vec2(texcoords.x - x, texcoords.y + y)).rgb;
    vec3 b = texture(samplerTexture, vec2(texcoords.x,     texcoords.y + y)).rgb;
    vec3 c = texture(samplerTexture, vec2(texcoords.x + x, texcoords.y + y)).rgb;

    vec3 d = texture(samplerTexture, vec2(texcoords.x - x, texcoords.y)).rgb;
    vec3 e = texture(samplerTexture, vec2(texcoords.x,     texcoords.y)).rgb;
    vec3 f = texture(samplerTexture, vec2(texcoords.x + x, texcoords.y)).rgb;

    vec3 g = texture(samplerTexture, vec2(texcoords.x - x, texcoords.y - y)).rgb;
    vec3 h = texture(samplerTexture, vec2(texcoords.x,     texcoords.y - y)).rgb;
    vec3 i = texture(samplerTexture, vec2(texcoords.x + x, texcoords.y - y)).rgb;

    // Apply weighted distribution, by using a 3x3 tent filter:
    //  1   | 1 2 1 |
    // -- * | 2 4 2 |
    // 16   | 1 2 1 |

    vec3 upsample = vec3(0.0);

    upsample = e*4.0;
    upsample += (b+d+f+h)*2.0;
    upsample += (a+c+g+i);
    upsample *= 1.0 / 16.0;

    return upsample;
}

vec3 bloom()
{
    vec3 hdrColor = texture(u_scene, TexCoords).rgb;
    vec3 bloomColor = upsample(u_bloomBlur, TexCoords, uniforms.resolution.x / uniforms.resolution.y);//, uniforms.TEST_SLIDER);

    // vec3 bloomColor = texture(u_bloomBlur, TexCoords).rgb;
    return hdrColor + bloomColor;// * u_bloomStrength; // additive blending
}

void main()
{
    vec3 result = bloom();
    FragColor = vec4(result, 1.0);
}
