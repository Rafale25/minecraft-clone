#version 460 core

in vec3 fragPos;

layout (location = 0) out vec4 FragColor;

#include "uniforms.glsl"
#include "skyColor.glsl"

vec3 sunColor = vec3(1.0, 0.9, 0.7);

void main()
{
    vec3 ray = normalize(fragPos);
    vec3 color = getSkyColor(ray, uniforms.sunDotAngle);

    mixSunColor(color, sunColor, ray, uniforms.sunDirection.xyz);

    FragColor = vec4(color, 1.0);
}
