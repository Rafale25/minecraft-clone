#pragma once

#include "Framebuffer.hpp"
#include "Texture.hpp"
#include <glm/detail/type_mat4x4.hpp>

class Camera;
class Context;
class Program;

struct FrustumBounds {
    float minX, maxX;
    float minY, maxY;
    float minZ, maxZ;
};

class Shadowmap
{
public:
    Shadowmap(GLsizei shadowmap_size);

    glm::mat4 begin(const glm::mat4& projection, const glm::mat4& view, const Program &program);

    void end();
    void setSunDir(const glm::vec3& sunDir);

private:
    glm::mat4 getLightViewMatrix(const std::vector<glm::vec3>& cameraFrustumCorners, const glm::vec3& lightDir);
    FrustumBounds computeFrustumBounds(const glm::mat4& lightView, const std::vector<glm::vec3>& corners);
    glm::mat4 getLightProjectionMatrix(FrustumBounds& b);

    glm::mat4 getLightSpaceMatrix(const Camera& camera, float near_plane, float far_plane);
    std::vector<glm::mat4> getLightSpaceMatrices(const Camera& camera);

// private:
public:
    GLsizei _shadowmap_size;
    GLint _cached_viewport[4];
    Framebuffer _depthFBO{GL_NONE, GL_NONE};
    glm::vec3 _sunDir;

public:
    static constexpr float camera_far_plane = 1000.0f;
    std::array<float, 4> shadowCascadeLevels{ camera_far_plane / 50.0f, camera_far_plane / 25.0f, camera_far_plane / 10.0f, camera_far_plane / 2.0f };

    // uniform sampler2DArray shadowMap;
    GLuint _depthTextureArray;
    GLuint _matricesBuffer;

    // Texture _depthTexture;
    glm::mat4 _lightSpaceMatrix;
    float _shadow_bias = 0.0005; // 0.0005 looks good for 4096
};
