#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>

#include "framebuffer.hpp"
#include "texture.hpp"

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
    Shadowmap(Context& ctx, GLsizei shadow_width, GLsizei shadow_height);

    void begin(Camera& camera, Program &program);
    void end();
    void setSunDir(glm::vec3 sunDir);

private:
    std::vector<glm::vec4> getFrustumCornersWorldSpace(const glm::mat4& proj, const glm::mat4& view);
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
    float _max_shadow_distance = 120.0f;
    float _shadow_bias = 0.000175f;
};
