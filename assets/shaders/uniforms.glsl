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
