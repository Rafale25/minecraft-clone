#include "shadowmapping.glsl"

// https://github.com/HigashiSan/CDC-High-Quality-Realtime-Cloud
// Henyey-Greenstein function
float HenyeyGreensteinPhase(float angle, float g)
{
    float gg = g * g;
    return (1.0 - gg) / (4.0 * 3.14159 * pow(1.0 + gg - 2.0 * g * angle, 1.5));
}

float beersLaw(float dist, float absorption) {
    return exp(-dist * absorption);
}

float raymarchVolumetricLighting(vec3 end, float density=0.05, float volumetricHGphaseFront=0.65, float volumetricHGphaseBack=-0.36, float ambiantLight=0.01, float maxDistance=100.0)
{
    vec3 startPos = uniforms.viewPosition.xyz;
    vec3 endPos = end;

    vec3 ray = endPos - startPos;
    vec3 rayDirection = normalize(ray);
    float rayLength = clamp(length(ray), 0, maxDistance);

    int steps = 32;
    float stepSize = rayLength / steps;
    vec3 stepVec = rayDirection * stepSize;

    vec3 currentPos = startPos + rayDirection * rand(gl_FragCoord.xy * 0.01) * (stepSize*2);

    float accumulatedLight = 0.0;
    float transmittance = 1.0;

    const float cosTheta = dot(-uniforms.sunDirection.xyz, -rayDirection);
    const float scatterPhaseFront = HenyeyGreensteinPhase(cosTheta, volumetricHGphaseFront); // 0.65
    const float scatterPhaseBack = HenyeyGreensteinPhase(cosTheta, volumetricHGphaseBack); // -0.36
    const float scatterPhase = mix(scatterPhaseFront, scatterPhaseBack, 0.5);

    for (int i = 0 ; i < steps - 1; ++i) {
        transmittance *= beersLaw(density * stepSize, 1.0);

        bool lit = !isInShadow(u_shadowmap, uniforms.view, currentPos, uniforms.cascadeCount);
        float light = max(ambiantLight, float(lit));

        float luminance = light * scatterPhase * density;
        accumulatedLight += transmittance * luminance * stepSize;

        if (transmittance < 0.01) break;

        currentPos += stepVec;
    }

    return accumulatedLight;
}
