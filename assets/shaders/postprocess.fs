#version 460 core

in vec2 TexCoords;

out vec4 FragColor;

uniform mat4 u_view;
uniform mat4 u_projection;
uniform vec2 u_resolution;
uniform float u_FOV;
uniform float u_sunDotAngle;
uniform vec3 u_sunDirection;
uniform vec3 u_viewPosition;

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
    vec3 tint = vec3(1);
    vec3 skyColorMorning = SkyColourMorning(ray.xyz);
    vec3 skyColorZenit = SkyColour(ray.xyz);

    // vec3 color = mix(skyColorMorni:ng, skyColorZenit, clamp(u_sunDotAngle, 0.0, 1.0));
    vec3 color = mix(skyColorZenit, skyColorZenit, clamp(u_sunDotAngle, 0.0, 1.0));
    color *= tint;

    // corrections
    color = 0.6 + (clamp(color, 0.0, 1.0) - 0.6);
    color = pow(color, vec3(1.0/2.2));

    return color;
}

const float fog_start = 150.0;
const float fog_end = 350.0;
const vec3 fog_color = vec3(0.8);

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
              vec3 sunColor
){
    float b = 0.003;
    float fogAmount = 1.0 - exp(-t*b * t*b);
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
    vec4 viewSpace = inverse(u_projection) * clipSpace;
    return length(viewSpace.xyz / viewSpace.w);
}

void main()
{
    vec2 uv = (gl_FragCoord.xy - 0.5*u_resolution.xy) / u_resolution.y;

    vec3 color = texture(colorTexture, TexCoords).rgb;
    float depth = texture(depthTexture, TexCoords).r;
    vec3 worldPos = texture(worldPosTexture, TexCoords).rgb;

    vec2 what_uv = vec2(uv.x * 0.5 + 0.5, uv.y + 0.5);

    vec3 ray = normalize(mat3(inverse(u_view)) * skyray(what_uv, u_FOV, u_resolution.x / u_resolution.y));
    vec3 skyColor = getSkyColor(ray);

    // float fragDistance = depthToFragDistance(uv, depth);
    float fragDistance = distance(u_viewPosition, worldPos); //worldPosdepthToFragDistance(uv, depth);

    vec3 rd = normalize(worldPos - u_viewPosition);

    // color = mix(skyColor, color, calcExpFogFactor(fragDistance));
    // color = applyFog(color, fragDistance, rd, u_sunDirection);
    vec3 sunColor = vec3(1.0, 0.9, 0.7);
    color = applyFog(color, fragDistance, rd, u_sunDirection, skyColor, sunColor);

    // float b = 0.001;
    // color = color*exp(-fragDistance*b) + skyColor*(1.0-exp(-fragDistance*b));

    if (depth == 1.0) // is skybox
    {
        // reusing code from applyFog() function
        float sunAmount = max( dot(ray, u_sunDirection), 0.0 );
        vec3  finalfogColor  = mix( skyColor, sunColor, pow(sunAmount, 4.0) );

        FragColor = vec4(finalfogColor, 1.0);
    }
    else
    {
        FragColor = vec4(color, 1.0);
        // FragColor = vec4(worldPos, 1.0);
    }
}
