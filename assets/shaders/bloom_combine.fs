#version 460 core

out vec4 FragColor;

in vec2 TexCoords;

layout (location = 0) uniform sampler2D u_scene;
layout (location = 1) uniform sampler2D u_bloomBlur;

// uniform float u_bloomStrength = 1.0;

// vec3 bloom_none()
// {
//     vec3 hdrColor = texture(u_scene, texCoord).rgb;
//     return hdrColor;
// }

vec3 bloom()
{
    vec3 hdrColor = texture(u_scene, TexCoords).rgb;
    vec3 bloomColor = texture(u_bloomBlur, TexCoords).rgb;
    return hdrColor + bloomColor;// * u_bloomStrength; // additive blending
}

void main()
{
    vec3 result = vec3(0.0);
    result = bloom();

    FragColor = vec4(result, 1.0);
}
