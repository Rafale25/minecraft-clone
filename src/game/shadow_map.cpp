#include "shadow_map.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include "context.hpp"
#include "program.h"
#include "camera.hpp"
#include "texture.hpp"
#include "framebuffer.hpp"

static const float borderColor[4] = {1.0f, 1.0f, 1.0f, 1.0f};

Shadowmap::Shadowmap(Context& ctx, GLsizei shadow_width, GLsizei shadow_height):
    _ctx(ctx),
    _shadow_width(shadow_width),
    _shadow_height(shadow_height),
    _depthTexture(Texture(shadow_width, shadow_height, GL_DEPTH_COMPONENT24, GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_BORDER, borderColor))
{
    _depthTexture.setSwizzle({ GL_RED, GL_RED, GL_RED, GL_ONE });
    _depthFBO.attachTexture(_depthTexture._texture, GL_DEPTH_ATTACHMENT);
}

void Shadowmap::begin(const Camera& camera, const Program &program)
{
    glm::mat4 cameraCustomProj = glm::perspective(glm::radians(camera.fov), camera.aspect_ratio, 0.3f, _max_shadow_distance);
    auto corners = getFrustumCornersWorldSpace(cameraCustomProj, camera.getView());

    glm::mat4 lightViewMatrix = getLighViewMatrix(corners, _sunDir);
    FrustumBounds bounds = computeFrustumBounds(lightViewMatrix, corners);
    glm::mat4 lightProjectionMatrix = getLightProjectionMatrix(lightViewMatrix, bounds);
    _lightSpaceMatrix = lightProjectionMatrix * lightViewMatrix;
    // https://learn.microsoft.com/en-us/windows/win32/dxtecharts/common-techniques-to-improve-shadow-depth-maps?redirectedfrom=MSDN
    // https://chetanjags.wordpress.com/2015/02/05/real-time-shadows-cascaded-shadow-maps/
    // https://stackoverflow.com/questions/33499053/cascaded-shadow-map-shimmering

    program.use();
    program.setMat4("u_lightSpaceMatrix", _lightSpaceMatrix);
    glViewport(0, 0, _shadow_width, _shadow_height);
    _depthFBO.bind();
    glClear(GL_DEPTH_BUFFER_BIT);
}

void Shadowmap::end()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, _ctx.width, _ctx.height);
}

void Shadowmap::setSunDir(const glm::vec3& sunDir)
{
    _sunDir = sunDir;
}

std::vector<glm::vec4> Shadowmap::getFrustumCornersWorldSpace(const glm::mat4& proj, const glm::mat4& view)
{
    const auto inv = glm::inverse(proj * view);

    std::vector<glm::vec4> frustumCorners;
    for (unsigned int x = 0; x < 2; ++x) {
        for (unsigned int y = 0; y < 2; ++y) {
            for (unsigned int z = 0; z < 2; ++z) {
                const glm::vec4 pt =
                    inv * glm::vec4(
                        2.0f * x - 1.0f,
                        2.0f * y - 1.0f,
                        2.0f * z - 1.0f,
                        1.0f);
                frustumCorners.push_back(pt / pt.w);
            }
        }
    }

    return frustumCorners;
}

glm::mat4 Shadowmap::getLighViewMatrix(const std::vector<glm::vec4>& cameraFrustumCorners, const glm::vec3& lightDir)
{
    glm::vec3 center = glm::vec3(0, 0, 0);
    for (const auto& v : cameraFrustumCorners) {
        center += glm::vec3(v);
    }
    center /= cameraFrustumCorners.size();

    return glm::lookAt(
        center + lightDir,
        center,
        glm::vec3(0.0f, 1.0f, 0.0f)
    );
}

FrustumBounds Shadowmap::computeFrustumBounds(const glm::mat4& lightView, const std::vector<glm::vec4>& corners)
{
    FrustumBounds b;

    b.minX = std::numeric_limits<float>::max();
    b.maxX = std::numeric_limits<float>::lowest();
    b.minY = std::numeric_limits<float>::max();
    b.maxY = std::numeric_limits<float>::lowest();
    b.minZ = std::numeric_limits<float>::max();
    b.maxZ = std::numeric_limits<float>::lowest();

    for (const auto& v : corners)
    {
        const glm::vec4 trf = lightView * v;
        b.minX = std::min(b.minX, trf.x);
        b.maxX = std::max(b.maxX, trf.x);
        b.minY = std::min(b.minY, trf.y);
        b.maxY = std::max(b.maxY, trf.y);
        b.minZ = std::min(b.minZ, trf.z);
        b.maxZ = std::max(b.maxZ, trf.z);
    }

    return b;
}

glm::mat4 Shadowmap::getLightProjectionMatrix(const glm::mat4& lightView, FrustumBounds& b)
{
    // Tune this parameter according to the scene
    const float zMult = 5.0f;
    if (b.minZ < 0)
        b.minZ *= zMult;
    else
        b.minZ /= zMult;

    if (b.maxZ < 0)
        b.maxZ /= zMult;
    else
        b.maxZ *= zMult;

    return glm::ortho(b.minX, b.maxX, b.minY, b.maxY, b.minZ, b.maxZ);
}
