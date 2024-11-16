#version 460 core
#extension GL_ARB_bindless_texture : require

float rand(vec2 co){ return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453); }

layout(binding = 0, std430) readonly buffer ssbo {
    sampler2D texture_handles[];
};

const vec3 orientation_normal_table[] = {
    vec3(0.0, 1.0, 0.0), // Top = 0
    vec3(0.0, -1.0, 0.0), // Bottom = 1
    vec3(0.0, 0.0, -1.0), // Front = 2
    vec3(0.0, 0.0, 1.0), // Back = 3
    vec3(-1.0, 0.0, 0.0), // Left = 4
    vec3(1.0, 0.0, 0.0), // Right = 5
};

in VS_OUT {
    in vec3 frag_pos;
    in vec2 uv;
    flat in uint orientation;
    flat in uint texture_id;
    float ambient_occlusion;
    in vec4 FragPosLightSpace;
} fs_in;

out vec4 FragColor;

uniform vec3 u_sun_direction;
uniform vec3 u_view_position;
uniform float u_shadow_bias;
uniform bool u_ambient_occlusion_enabled;
uniform mat4 u_lightSpaceMatrix;

uniform sampler2D shadowMap;

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;

    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r;

    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    if (currentDepth > 1.0) {
        return 0.0;
    }

    // calculate bias (based on depth map resolution and slope)
    float cosTheta = dot(normal, normalize(u_sun_direction));
    float magic_bias_constant = u_shadow_bias;// 0.00035;
    float bias = magic_bias_constant*tan(acos(cosTheta));

    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y)
        {
            vec2 offset = vec2(x, y) + rand(projCoords.xy + vec2(x, y)); // smooth out shadows by using random offsets
            float pcfDepth = texture(shadowMap, projCoords.xy + offset * texelSize).r;
            shadow += (currentDepth - bias) > pcfDepth  ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if (projCoords.z > 1.0)
        shadow = 0.0;

    return shadow;
}

float shadowFast(vec4 lightSpacePos)
{
  vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;
  if (projCoords.z > 1.0) return 1.0;
  projCoords = projCoords * 0.5 + 0.5;
  if (any(lessThan(projCoords.xy, vec2(0))) || any(greaterThan(projCoords.xy, vec2(1)))) return 1.0;
  float closestDepth = texture(shadowMap, projCoords.xy).r;

  float currentDepth = projCoords.z;
  float shadow = currentDepth > closestDepth ? 1.0 : 0.0;

  return shadow;
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

float calcExpFogFactor()
{
    const float exp_fog_density = 0.2;
    float camera_to_pixel_dist = length(fs_in.frag_pos - u_view_position);
    float dist_ratio = 4.0 * camera_to_pixel_dist / fog_end;
    float fog_factor = exp(-dist_ratio*exp_fog_density * dist_ratio*exp_fog_density);
    return fog_factor;
}

float volumetric_light()
{
    #define STEP_SIZE 0.3
    #define MAX_STEPS 32

    const vec3 start = u_view_position;
    const vec3 end = fs_in.frag_pos;
    const float len = distance(start, end);

    // const float STEP_SIZE = len / MAX_STEPS;

    const vec3 dir = normalize(fs_in.frag_pos - u_view_position);
    float dist = rand(fs_in.frag_pos.xz) * STEP_SIZE;

    float accum = 0.0;

    int step = 0;
    while (step < MAX_STEPS && dist < len - 0.001) {
        step += 1;
        dist += STEP_SIZE;
        vec3 p = start + dir * dist;

        accum += 1.0 - shadowFast(u_lightSpaceMatrix * vec4(p, 1.0));
    }

    const float u_beer_power = 1.0;
    const float u_powder_power = 1.0;
    const float intensity = 0.025;

    const float d = accum * STEP_SIZE * intensity;
    const float powder = 1.0 - exp(-d * 2.0 * u_powder_power);
    const float beer = exp(-d * u_beer_power);

    return (1.0 - beer) * powder;
}

void main()
{
    vec4 color = texture(sampler2D(texture_handles[fs_in.texture_id]), fs_in.uv).rgba;
    vec3 normal = orientation_normal_table[fs_in.orientation];
    vec3 lightColor = vec3(255.0, 244.0, 196.0) / 255.0;

    // ambient
    float ambientStrength = 0.35;
    vec3 ambient = ambientStrength * lightColor;

    // diffuse
    float diff = max(dot(normal, normalize(u_sun_direction)), 0.0);
    vec3 diffuse = diff * lightColor;

    if (color.a < 0.65) { // magic value
        discard;
    }

    // calculate shadow
    float shadow = ShadowCalculation(fs_in.FragPosLightSpace, normal);

    // if cube face is not facing light, then it's in its own shadow
    if (dot(normal, u_sun_direction) < 0.0
    || u_sun_direction.y < 0.0) // if sun is under the ground (points up)
        shadow = 1.0;

    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse)) * color.rgb;

    if (u_ambient_occlusion_enabled) {
        lighting *= sqrt(fs_in.ambient_occlusion);
    }

    // Fog
    // lighting = mix(fog_color, lighting, calcExpFogFactor());

    float vlight = volumetric_light();
    lighting += vlight;

    FragColor = vec4(lighting, 1.0);

    // vec3 gammaCorrected = pow(lighting, vec3(1.0/2.2));
    // FragColor = vec4(gammaCorrected, 1.0);
    // FragColor = vec4(normal, 1.0);
}
