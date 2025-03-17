#version 460 core
#extension GL_ARB_bindless_texture : require

float rand(vec2 co){ return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453); }

layout(std430, binding = 0) readonly buffer ssbo_texture_handles {
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

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec3 gPosition;

uniform vec3 u_sun_direction;
uniform float u_shadow_bias;
uniform bool u_ambient_occlusion_enabled = true;
uniform float u_ambient_occlusion_strength = 0.9;
uniform vec2 u_resolution;

uniform vec3 u_view_position;
uniform vec3 u_view_dir;

uniform sampler2D shadowMap;

float sdSphere( vec3 p, float s ) {
    return length(p)-s;
}

float sdTorus( vec3 p, vec2 t )
{
  vec2 q = vec2(length(p.xz)-t.x,p.y);
  return length(q)-t.y;
}

mat4 rotation3d(vec3 axis, float angle) {
  axis = normalize(axis);
  float s = sin(angle);
  float c = cos(angle);
  float oc = 1.0 - c;

  return mat4(
    oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
    oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
    oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
    0.0,                                0.0,                                0.0,                                1.0
  );
}


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

vec3 CalcLight(vec3 _diffuse, vec3 _specular, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 light_position = u_view_position + vec3(50.0, 0.0, 0.0);
    vec3 light_color = vec3(1.0, 0.2, 0.1) * 2;
    float light_linear = 0.09;
    float light_quadratic = 0.032;

    vec4 p = rotation3d(vec3(0, 0, 1), 3.14*0.5) * vec4(fragPos - light_position, 1.0);
    float dist = sdTorus(p.xyz, vec2(20.0, 0.2));
    // if (dist < 0) return vec3(100, 0, 0);
    dist = max(0.0, dist);

    // diffuse
    vec3 lightDir = normalize(light_position - fragPos);
    vec3 diffuse = _diffuse * light_color;
    // attenuation
    float attenuation = 1.0 / (1.0 + light_linear * dist + light_quadratic * dist * dist);
    diffuse *= attenuation;
    return diffuse;
}

void main()
{
    vec2 uv = (gl_FragCoord.xy - 0.5*u_resolution.xy) / u_resolution.y;

    vec4 color = texture(texture_handles[fs_in.texture_id], fs_in.uv).rgba;
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

    lighting += CalcLight(color.rgb, vec3(0.0, 0.0, 0.0), normal, fs_in.frag_pos, u_view_dir);

    if (u_ambient_occlusion_enabled) {
        lighting = mix(lighting * (1.0 - u_ambient_occlusion_strength), lighting, fs_in.ambient_occlusion);
    }

    FragColor = vec4(lighting, 1.0);
    gPosition = fs_in.frag_pos;
    // FragColor = vec4(fs_in.frag_pos, 1.0);

    // vec3 gammaCorrected = pow(lighting, vec3(1.0/2.2));
    // FragColor = vec4(gammaCorrected, 1.0);
    // FragColor = vec4(normal, 1.0);
}
