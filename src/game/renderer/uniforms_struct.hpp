#pragma once

#include <glm/detail/type_mat4x4.hpp>
#include <glm/detail/type_vec2.hpp>

struct uniformsParameters {
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
    float volumetricHGphasePower = 0.0f;
    int ambient_occlusion_enabled = 0;
    int tonemapping_enabled = 0;
    int cascadeCount = 0;
};
