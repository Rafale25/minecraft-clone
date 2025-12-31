#include "shadowmapping.glsl"

// https://blog.maximeheckel.com/posts/shaping-light-volumetric-lighting-with-post-processing-and-raymarching/
// Henyey-Greenstein function
float hgPhase(float mu, float g) {
      float _g = g;//SCATTERING_ANISO;
      float _gg = g * g;

    float denom = 1.0 + _gg - 2.0 * _g * mu;
    denom = max(denom, 0.0001);

    float scatter = (1.0 - _gg) / pow(denom, 1.5);
    return scatter;
}

float raymarchVolumetricLighting(vec3 end, float density=0.05, float g=0.555, float maxDistance=100.0)//, float depth)
{
    vec3 startPos = uniforms.viewPosition.xyz;
    vec3 endPos = end;
    // if (depth == 1.0) {
    //     endPos = uniforms.viewPosition
    // }

    vec3 ray = endPos - startPos;
    vec3 rayDirection = normalize(ray);
    float rayLength = clamp(length(ray), 0, maxDistance);

    int steps = 32;
    float stepSize = rayLength / steps;
    vec3 stepVec = rayDirection * stepSize;

    vec3 currentPos = startPos + rayDirection * rand(gl_FragCoord.xy * 0.01) * (stepSize*2);

    float L = 0.0;
    float T = 1.0;

    // float accumulatedLight = 0.0;
    for (int i = 0 ; i < steps - 1; ++i) {
        float stepTransmittance = exp(-density * stepSize);

        bool lit = !isInShadow(u_shadowmap, uniforms.view, currentPos);
        // accumulatedLight += float(lit);

        if (lit) {
            float cosTheta = dot(-uniforms.sunDirection.xyz, -rayDirection);
            // float phase = hgPhase(cosTheta, 0.6);
            float phase = hgPhase(cosTheta, g);

            float scatteredLight = 1.0 * phase * density;

            // Only the light that survives so far contributes
            L += T * scatteredLight * (1.0 - stepTransmittance);
        }

        // T *= stepTransmittance;

        if (T < 0.01) break;

        currentPos += stepVec;
    }

    // accumulatedLight /= steps;

    return L;
}
