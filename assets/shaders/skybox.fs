#version 460 core

#define GROUND false

out vec4 FragColor;

#include "uniforms.glsl"
#include "skyColor.glsl"

void main()
{
    vec2 uv = (gl_FragCoord.xy - 0.5*uniforms.resolution.xy) / uniforms.resolution.y;
    vec3 ray = mat3(inverse(uniforms.view)) * skyray(uv + 0.5, uniforms.FOV, uniforms.resolution.x / uniforms.resolution.y);

    vec3 color = getSkyColor(ray, uniforms.sunDotAngle);

    FragColor = vec4(color, 1.0);
    // FragColor = vec4(ray, 1.0);
}
