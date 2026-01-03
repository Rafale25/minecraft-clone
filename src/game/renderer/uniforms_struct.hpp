#pragma once

#include <glm/detail/type_mat4x4.hpp>
#include <glm/detail/type_vec2.hpp>

#define MAT4 glm::mat4
#define VEC4 glm::vec4
#define VEC2 glm::vec2
#define INIT(x) {x}

struct uniformsParameters {
    #include "uniforms_raw"
};

/*
    glm::mat4 projection = glm::mat4(1.0f);
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection_view = glm::mat4(1.0f);
    glm::mat4 lightSpaceMatrix = glm::mat4(1.0f);
    glm::mat4 lightSpaceMatrices[4] = {}; // cascaded shadowmapping
    glm::vec4 sunDirection = glm::vec4(0.0f);
    glm::vec4 viewPosition = glm::vec4(0.0f);
    glm::vec4 cascadePlaneDistances = glm::vec4(0.0f);
    glm::vec2 resolution = glm::vec2(0.0f);
    // glm::vec2 resolutionInverse;
    float sunDotAngle = 0.0f;
    float FOV = 0.0f;
    float fogDensity = 0.0f;
    float shadow_bias = 0.0f;
    float ambient_occlusion_strength = 0.0f;
    float time = 0.0f;
    float exposure = 0.0f;
    float volumetricDensity = 0.0f;
    float volumetricHGphaseFront = 0.0f;
    float volumetricHGphaseBack = 0.0f;
    float volumetricAmbiantLight = 0.0f;
    int ambient_occlusion_enabled = 0;
    int tonemapping_enabled = 0;
    int cascadeCount = 0;
    int shadows_enabled = 0;
    float TEST_SLIDER_0 = 0.0f;
    float TEST_SLIDER_1 = 0.0f;
*/
