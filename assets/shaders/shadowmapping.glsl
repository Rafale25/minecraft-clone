#include "utils/random.glsl"

layout(std140, binding = 1) uniform uniformBufferMatrices {
    mat4 u_lightSpaceMatrices[4]; // 16
};

uniform float u_cascadePlaneDistances[4];
const int cascadeCount = 4;

float getSlopeScaledBias(vec3 N, vec3 L)
{
    float cosAlpha = clamp(dot(N, L), 0.0, 1.0);
    float sinAlpha = sqrt(1.0 - cosAlpha * cosAlpha);     // sin(acos(L*N))
    float tanAlpha = sinAlpha / cosAlpha;            // tan(acos(L*N))
    return tanAlpha;
}


int getShadowMapLayer(mat4 viewMatrix, vec3 fragWorldPos)
{
    vec4 fragPosViewSpace = viewMatrix * vec4(fragWorldPos, 1.0);
    float depthValue = abs(fragPosViewSpace.z);
    // float depthValue = distance(fragWorldPos, uniforms.viewPosition.xyz);

    int layer = -1;
    for (int i = 0; i < cascadeCount; ++i)
    {
        if (depthValue < u_cascadePlaneDistances[i])
        {
            layer = i;
            break;
        }
    }
    if (layer == -1)
    {
        layer = cascadeCount;
    }

    return layer;
}

float ShadowCalculation(sampler2DArray shadowMap, mat4 viewMatrix, vec3 fragWorldPos, vec3 normal, vec3 lightDirection, float shadowBias)
{
    int layer = getShadowMapLayer(viewMatrix, fragWorldPos);

    vec4 fragPosLightSpace = u_lightSpaceMatrices[layer] * vec4(fragWorldPos, 1.0);

    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;

    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    // float closestDepth = texture(shadowMap, vec3(projCoords.xy, 0)).r;

    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    if (currentDepth > 1.0) {
        return 0.0;
    }

    // calculate bias (based on slope and sunDirection)
    float bias = shadowBias * getSlopeScaledBias(normal, lightDirection);

    // const float biasModifier = 0.5;
    // const float farPlane = 1000.0;
    // if (layer == cascadeCount)
    // {
    //     bias *= 1.0 / (farPlane * biasModifier);
    // }
    // else
    // {
    //     bias *= 1.0 / (u_cascadePlaneDistances[layer] * biasModifier);
    // }

    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0).xy;
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y)
        {
            // vec2 offset = vec2(0.0);
            vec2 offset = vec2(x, y) + rand(projCoords.xy + vec2(x, y)); // smooth out shadows by using random offsets
            float pcfDepth = texture(shadowMap, vec3(projCoords.xy + offset * texelSize, layer)).r;
            shadow += (currentDepth - bias) > pcfDepth  ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if (projCoords.z > 1.0)
        shadow = 0.0;

    return shadow;
}

bool isInShadow(sampler2DArray shadowMap, mat4 viewMatrix, vec3 fragWorldPos)
{
    int layer = getShadowMapLayer(viewMatrix, fragWorldPos);

    vec4 fragPosLightSpace = u_lightSpaceMatrices[layer] * vec4(fragWorldPos, 1.0);

    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;

    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if (projCoords.z > 1.0)
        return false;

    float depth = texture(shadowMap, vec3(projCoords.xy, layer)).r;
    const float bias = 0.00005;
    if (projCoords.z - bias > depth) {
        return true;
    }

    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if (projCoords.z > 1.0)
        return false;

    return false;
}
