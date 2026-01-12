#pragma once

#include "Framebuffer.hpp"
#include "Texture.hpp"
#include <glm/detail/type_mat4x4.hpp>
#include <array>

class Camera;
class Context;
class ShaderProgram;

struct FrustumBounds {
    float minX, maxX;
    float minY, maxY;
    float minZ, maxZ;
};

class Shadowmap
{
public:
    Shadowmap(GLsizei shadowmap_size);

    glm::mat4 begin(const glm::mat4& projection, const glm::mat4& view, const ShaderProgram &program, int32_t i);
    void end();
    void setSunDir(const glm::vec3& sunDir);
    int32_t getShadowmapSize() const { return _shadowmap_size; }

// private:
    glm::mat4 getLightSpaceMatrix(const glm::mat4& lightViewProjectionMatrix);
    std::vector<glm::mat4> getLightSpaceMatrices(const Camera& camera);

    GLsizei _shadowmap_size = 0;
    Framebuffer _depthFBO{GL_NONE, GL_NONE};
    glm::vec3 _sunDir{0.0f, 0.0f, 0.0f};
private:
    GLint _cached_viewport[4]{};

public:
    static constexpr float camera_far_plane = 1000.0f;

    std::array<float, 4> shadowCascadeLevels{
        20, 50, 200, camera_far_plane
        // camera_far_plane / 50.0f,
        // camera_far_plane / 25.0f,
        // camera_far_plane / 10.0f,
        // camera_far_plane / 1.0f
    };

    GLuint m_depthTextureArray = 0;
    glm::mat4 m_lightSpaceMatrix;
    float m_shadowBias = 0.0005; // 0.0005 looks good for 4096
};
