#pragma once

#include <glm/detail/type_mat4x4.hpp>

#include "Framebuffer.hpp"
#include "Texture.hpp"

class Camera;
class Context;
class Program;
// struct AABB;

struct FrustumBounds {
    float minX, maxX;
    float minY, maxY;
    float minZ, maxZ;
};

class Shadowmap
{
public:
    // TODO: Remove ctx from shadowmap
    Shadowmap(Context& ctx, GLsizei shadow_width, GLsizei shadow_height);

    glm::mat4 begin(const glm::mat4& projection, const glm::mat4& view, const Program &program);

    void end();
    void setSunDir(const glm::vec3& sunDir);

private:
    glm::mat4 getLighViewMatrix(const std::vector<glm::vec4>& cameraFrustumCorners, const glm::vec3& lightDir);
    FrustumBounds computeFrustumBounds(const glm::mat4& lightView, const std::vector<glm::vec4>& corners);
    glm::mat4 getLightProjectionMatrix(const glm::mat4& lightView, FrustumBounds& b);

private:
    Context& _ctx;
    GLsizei _shadow_width, _shadow_height;
    Framebuffer _depthFBO{GL_NONE, GL_NONE};
    glm::vec3 _sunDir;

public:
    Texture _depthTexture;
    glm::mat4 _lightSpaceMatrix;
    float _shadow_bias = 0.000175f;
};
