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
    glm::mat4 getLighViewMatrix(const std::vector<glm::vec3>& cameraFrustumCorners, const glm::vec3& lightDir);
    FrustumBounds computeFrustumBounds(const glm::mat4& lightView, const std::vector<glm::vec3>& corners);
    glm::mat4 getLightProjectionMatrix(FrustumBounds& b);

// private:
public:
    GLsizei _shadowmap_size;
    GLint _cached_viewport[4];
    Framebuffer _depthFBO{GL_NONE, GL_NONE};
    glm::vec3 _sunDir;

public:
    Texture _depthTexture;
    glm::mat4 _lightSpaceMatrix;
    float _shadow_bias = 0.00016f;
};
