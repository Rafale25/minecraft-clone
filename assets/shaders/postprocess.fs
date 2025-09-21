#version 460 core

in vec2 TexCoords;

out vec4 FragColor;

layout (location = 0) uniform sampler2D colorTexture;
layout (location = 1) uniform sampler2D worldPosTexture;
layout (location = 2) uniform sampler2D depthTexture;

#include "uniforms.glsl"
#include "skyColor.glsl"

// const float fog_start = 150.0;
// const float fog_end = 350.0;
// const vec3 fog_color = vec3(0.8);

// float calcLinearFogFactor()
// {
//     float camera_to_pixel_dist = length(fs_in.frag_pos - u_view_position);
//     float fog_range = fog_end - fog_start;
//     float fog_dist = fog_end - camera_to_pixel_dist;
//     float fog_factor = fog_dist / fog_range;
//     fog_factor = clamp(fog_factor, 0.0, 1.0);
//     return fog_factor;
// }

// float calcExpFogFactor(float dist)
// {
//     const float exp_fog_density = 0.2;
//     float camera_to_pixel_dist = dist;//length(fs_in.frag_pos - u_view_position);
//     float dist_ratio = 4.0 * camera_to_pixel_dist / fog_end;
//     float fog_factor = exp(-dist_ratio*exp_fog_density * dist_ratio*exp_fog_density);
//     return fog_factor;
// }

vec3 applyFog(vec3  col,   // color of pixel
              float t,     // distance to point
              vec3  rd,    // camera to point
              vec3  lig,   // sun direction
              vec3 fogColor,
              vec3 sunColor,
              float fogDensity // = 0.003
){
    float fogAmount = 1.0 - exp(-t*fogDensity * t*fogDensity);
    float sunAmount = max( dot(rd, lig), 0.0 );
    vec3  finalfogColor  = mix( fogColor, sunColor, pow(sunAmount, 8.0) );
    // vec3  finalfogColor  = mix( vec3(0.5,0.6,0.7), // blue
    //                        vec3(1.0,0.9,0.7), // yellow
    //                        pow(sunAmount, 8.0) );
    return mix( col, finalfogColor, fogAmount );
}

void main()
{
    vec2 uv = (gl_FragCoord.xy - 0.5*uniforms.resolution.xy) / uniforms.resolution.y;

    vec3 color = texture(colorTexture, TexCoords).rgb;
    float depth = texture(depthTexture, TexCoords).r;
    vec3 worldPos = texture(worldPosTexture, TexCoords).rgb;

    vec2 what_uv = vec2(uv.x * 0.5 + 0.5, uv.y + 0.5);

    vec3 ray = normalize(mat3(inverse(uniforms.view)) * skyray(what_uv, uniforms.FOV, uniforms.resolution.x / uniforms.resolution.y));
    vec3 skyColor = getSkyColor(ray, uniforms.sunDotAngle);

    // float fragDistance = depthToFragDistance(uv, depth);
    float fragDistance = distance(uniforms.viewPosition.xyz, worldPos); //worldPosdepthToFragDistance(uv, depth);

    vec3 rd = normalize(worldPos - uniforms.viewPosition.xyz);

    // color = mix(skyColor, color, calcExpFogFactor(fragDistance));
    // color = applyFog(color, fragDistance, rd, u_sunDirection);
    vec3 sunColor = vec3(1.0, 0.9, 0.7);
    color = applyFog(color, fragDistance, rd, uniforms.sunDirection.xyz, skyColor, sunColor, uniforms.fogDensity);

    // float b = 0.001;
    // color = color*exp(-fragDistance*b) + skyColor*(1.0-exp(-fragDistance*b));

    vec4 finalColor;

    if (depth == 1.0) // is skybox
    {
        // reusing code from applyFog() function
        float sunAmount = max( dot(ray, uniforms.sunDirection.xyz), 0.0 );
        vec3 finalfogColor  = mix( skyColor, sunColor, pow(sunAmount, 4.0) );

        // sun
        float sun = pow(max(0.0, dot(ray, normalize(uniforms.sunDirection.xyz))), 4096.0) * 1.0;
        float groundToSkyT = smoothstep(-0.1, 0.0, ray.y);
        float sunMask = float(groundToSkyT >= 1.0);
        finalfogColor += sun * groundToSkyT;

        finalColor = vec4(finalfogColor, 1.0);
    }
    else
    {
        //
        float night_factor = 2.0;
        float steepness = 8.0;
        float ambiant_factor = 1.0 - pow(2.0, -steepness*uniforms.sunDotAngle - night_factor);
        ambiant_factor = clamp(ambiant_factor, 0.25, 1.0);

        finalColor = vec4(color * ambiant_factor, 1.0);
        // FragColor = vec4(worldPos, 1.0);
    }

    FragColor = finalColor;
}
