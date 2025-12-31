#pragma once

#include <glm/detail/type_mat4x4.hpp>
#include <glm/detail/type_vec2.hpp>

struct uniformsParameters {
    glm::mat4 projection;
    glm::mat4 view;
    glm::mat4 projection_view;
    glm::mat4 lightSpaceMatrix;
    glm::vec4 sunDirection;
    glm::vec4 viewPosition;
    glm::vec4 cascadePlaneDistances;
    glm::vec2 resolution;
    // glm::vec2 resolutionInverse;
    float sunDotAngle;
    float FOV;
    float fogDensity;
    float shadow_bias;
    float ambient_occlusion_strength;
    float time;
    float exposure;
    int ambient_occlusion_enabled;
    int tonemapping_enabled;
};
