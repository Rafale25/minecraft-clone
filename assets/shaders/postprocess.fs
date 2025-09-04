#version 460 core

in vec2 TexCoords;

out vec4 FragColor;

#include "uniforms.glsl"


layout (location = 0) uniform sampler2D colorTexture;
layout (location = 1) uniform sampler2D worldPosTexture;
layout (location = 2) uniform sampler2D depthTexture;

// https://www.shadertoy.com/view/4ljBRy
// quick and pretty sky colour
vec3 SkyColour(vec3 ray)
{
    return exp2(-ray.y/vec3(.1,.3,.6)); // blue
//    return exp2(-ray.y/vec3(.18,.2,.28))*vec3(1,.95,.8); // overcast
//    return exp2(-ray.y/vec3(.1,.2,.8))*vec3(1,.75,.5); // dusk
//    return exp2(-ray.y/vec3(.03,.2,.9)); // tropical blue
//    return exp2(-ray.y/vec3(.4,.06,.01)); // orange-red
//    return exp2(-ray.y/vec3(.1,.2,.01)); // green
}

vec3 SkyColourMorning(vec3 ray)
{
   return exp2(-ray.y/vec3(.1,.2,.8))*vec3(1,.75,.5); // dusk
}

vec3 skyray(vec2 uv, float fieldOfView, float aspectRatio)
{
    float d = 0.5 / tan(fieldOfView / 2.0);
    return vec3((uv.x - 0.5) * aspectRatio, uv.y - 0.5, -d);
}

vec3 getSkyColor(vec3 ray) {
    // Dynamic horizon height based on camera.y
    // const float horizon_height = -u_viewPosition.y * 0.001; //-0.15;
    // ray = normalize(ray - vec3(0.0, horizon_height, 0.0));

    vec3 tint = vec3(1);
    vec3 skyColorMorning = SkyColourMorning(ray.xyz);
    vec3 skyColorZenit = SkyColour(ray.xyz);

    // vec3 color = mix(skyColorMorni:ng, skyColorZenit, clamp(uniforms.sunDotAngle, 0.0, 1.0));
    vec3 color = mix(skyColorMorning, skyColorZenit, clamp(uniforms.sunDotAngle, 0.0, 1.0));
    color *= tint;

    // corrections
    color = 0.6 + (clamp(color, 0.0, 1.0) - 0.6);
    color = pow(color, vec3(1.0/2.2));

    return color;
}

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

float linearize_depth(float d, float zNear, float zFar)
{
    return zNear * zFar / (zFar + d * (zNear - zFar));
}

float depthToFragDistance(vec2 uv, float depth)
{
    vec4 clipSpace = vec4(uv, depth*2.0-1.0, 1.0);
    vec4 viewSpace = inverse(uniforms.projection) * clipSpace;
    return length(viewSpace.xyz / viewSpace.w);
}

void main()
{
    vec2 uv = (gl_FragCoord.xy - 0.5*uniforms.resolution.xy) / uniforms.resolution.y;

    vec3 color = texture(colorTexture, TexCoords).rgb;
    float depth = texture(depthTexture, TexCoords).r;
    vec3 worldPos = texture(worldPosTexture, TexCoords).rgb;

    vec2 what_uv = vec2(uv.x * 0.5 + 0.5, uv.y + 0.5);

    vec3 ray = normalize(mat3(inverse(uniforms.view)) * skyray(what_uv, uniforms.FOV, uniforms.resolution.x / uniforms.resolution.y));
    vec3 skyColor = getSkyColor(ray);

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
