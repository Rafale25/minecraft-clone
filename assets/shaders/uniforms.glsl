layout(std430, binding=3) readonly buffer uniformsBuffer {
    mat4 projection;
    mat4 view;
    mat4 projection_view;
    mat4 lightSpaceMatrix;
    mat4 lightSpaceMatrices[4]; // cascaded shadowmapping
    vec4 sunDirection;
    vec4 viewPosition;
    float cascadePlaneDistances[4];
    vec2 resolution;
    float sunDotAngle;
    float FOV;
    float fogDensity;
    float shadow_bias;
    float ambient_occlusion_strength;
    float time;
    float exposure;
    float volumetricDensity;
    float volumetricHGphaseFront;
    float volumetricHGphaseBack;
    float volumetricAmbiantLight;
    int ambient_occlusion_enabled;
    int tonemapping_enabled;
    int cascadeCount;
    int shadows_enabled;
    float TEST_SLIDER_0;
    float TEST_SLIDER_1;
} uniforms;
