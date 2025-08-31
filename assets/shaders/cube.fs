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

layout(std140, binding = 0) uniform uniformBuffer {
    mat4 projection;
    mat4 view;
    mat4 projection_view;
    mat4 lightSpaceMatrix;
    vec4 sunDirection;
    vec4 viewPosition;
    vec2 resolution;
    float sunDotAngle;
    float FOV;
    float fogDensity;
    float shadow_bias;
    float ambient_occlusion_strength;
    float time;
    float exposure;
    int ambient_occlusion_enabled;
    int tonemapping_enabled;
} uniforms;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec3 gPosition;

// uniform vec3 u_sun_direction;
// uniform float u_shadow_bias;
// uniform bool u_ambient_occlusion_enabled = true;
// uniform float u_ambient_occlusion_strength = 0.9;
// uniform vec2 u_resolution;
// uniform bool u_tonemapping_enabled = true;
// uniform float u_exposure = 1.0;


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
    float cosTheta = dot(normal, normalize(uniforms.sunDirection.xyz));
    float magic_bias_constant = uniforms.shadow_bias;// 0.00035;
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

vec3 Uncharted2Tonemap(vec3 x) {
	float Brightness = 0.28;
	x*= Brightness;
	float A = 0.28;
	float B = 0.29;
	float C = 0.10;
	float D = 0.2;
	float E = 0.025;
	float F = 0.35;
	return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

vec3 unchartedTonemapping(vec3 color)
{
	vec3 curr = Uncharted2Tonemap(color*4.7);
	color = curr/Uncharted2Tonemap(vec3(15.2));
	return color;
}

vec3 PBRNeutralToneMapping( vec3 color ) {
  const float startCompression = 0.8 - 0.04;
  const float desaturation = 0.15;

  float x = min(color.r, min(color.g, color.b));
  float offset = x < 0.08 ? x - 6.25 * x * x : 0.04;
  color -= offset;

  float peak = max(color.r, max(color.g, color.b));
  if (peak < startCompression) return color;

  const float d = 1. - startCompression;
  float newPeak = 1. - d * d / (peak + d - startCompression);
  color *= newPeak / peak;

  float g = 1. - 1. / (desaturation * (peak - newPeak) + 1.);
  return mix(color, newPeak * vec3(1, 1, 1), g);
}

vec3 lottes(vec3 x) {
  x *= vec3(0.9); //I reduced the light a little
  const vec3 a = vec3(1.6);
  const vec3 d = vec3(0.977);
  const vec3 hdrMax = vec3(8.0);
  const vec3 midIn = vec3(0.18);
  const vec3 midOut = vec3(0.267);

  const vec3 b =
	  (-pow(midIn, a) + pow(hdrMax, a) * midOut) /
	  ((pow(hdrMax, a * d) - pow(midIn, a * d)) * midOut);
  const vec3 c =
	  (pow(hdrMax, a * d) * pow(midIn, a) - pow(hdrMax, a) * pow(midIn, a * d) * midOut) /
	  ((pow(hdrMax, a * d) - pow(midIn, a * d)) * midOut);

  return pow(x, a) / (pow(x, a * d) * b + c);
}

vec3 toLinearSRGB(vec3 sRGB)
{
	bvec3 cutoff = lessThan(sRGB, vec3(0.04045));
	vec3 higher = pow((sRGB + vec3(0.055))/vec3(1.055), vec3(2.4));
	vec3 lower = sRGB/vec3(12.92);
	return mix(higher, lower, cutoff);
}

vec3 fromLinearToSRGB(vec3 linearRGB)
{
	bvec3 cutoff = lessThan(linearRGB, vec3(0.0031308));
	vec3 higher = vec3(1.055)*pow(linearRGB, vec3(1.0/2.4)) - vec3(0.055);
	vec3 lower = linearRGB * vec3(12.92);

	return mix(higher, lower, cutoff);
}

void main()
{
    vec2 uv = (gl_FragCoord.xy - 0.5*uniforms.resolution.xy) / uniforms.resolution.y;

    // vec4 color = vec4(0.2, 1.0, 0.0, 1.0);
    vec4 color = texture(texture_handles[fs_in.texture_id], fs_in.uv).rgba;
    color.rgb = toLinearSRGB(color.rgb);// pow(color.rgb, vec3(2.2));

    vec3 normal = orientation_normal_table[fs_in.orientation];
    vec3 lightColor = vec3(255.0, 244.0, 196.0) / 255.0;

    // ambient
    float ambientStrength = 0.15;// 35;
    vec3 ambient = ambientStrength * lightColor;

    // diffuse
    float diff = max(dot(normal, normalize(uniforms.sunDirection.xyz)), 0.0);
    vec3 diffuse = diff * lightColor;

    if (color.a < 0.65) { // magic value
        discard;
    }

    // calculate shadow
    float shadow = ShadowCalculation(fs_in.FragPosLightSpace, normal);

    // if cube face is not facing light, then it's in its own shadow
    if (dot(normal, uniforms.sunDirection.xyz) < 0.0
    || uniforms.sunDirection.y < 0.0) // if sun is under the ground (points up)
        shadow = 1.0;

    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse)) * color.rgb;

    if (uniforms.ambient_occlusion_enabled == 1) {
        lighting = mix(lighting * (1.0 - uniforms.ambient_occlusion_strength), lighting, fs_in.ambient_occlusion);
    }

    // FragColor = vec4(lighting, color.a);
    gPosition = fs_in.frag_pos;
    // FragColor = vec4(fs_in.frag_pos, 1.0);

    if (uniforms.tonemapping_enabled == 1) {
        lighting = lottes(lighting.rgb * uniforms.exposure);
    }

    vec3 gammaCorrected = fromLinearToSRGB(lighting);// pow(lighting, vec3(1.0/2.2));
    FragColor = vec4(gammaCorrected, 1.0);

    // FragColor = vec4(normal, 1.0);
}
