#include "Shadowmap.hpp"
#include "Program.hpp"
#include "Frustum.hpp"
#include <glm/gtc/matrix_transform.hpp>

static const float borderColor[4] = {1.0f, 1.0f, 1.0f, 1.0f};

Shadowmap::Shadowmap(GLsizei shadowmap_size):
    _shadowmap_size(shadowmap_size),
    _depthTexture(Texture(shadowmap_size, shadowmap_size, GL_DEPTH_COMPONENT24, GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_BORDER, borderColor))
{
    _depthTexture.setSwizzle({ GL_RED, GL_RED, GL_RED, GL_ONE });
    _depthFBO.attachTexture(_depthTexture._texture, GL_DEPTH_ATTACHMENT);
}

glm::mat4 Shadowmap::begin(const glm::mat4& projection, const glm::mat4& view, const Program &program)
{
    auto corners = extractFrustumCornersWorldSpace(projection * view);

    glm::mat4 lightViewMatrix = getLighViewMatrix(corners, _sunDir);
    FrustumBounds bounds = computeFrustumBounds(lightViewMatrix, corners);
    glm::mat4 lightProjectionMatrix = getLightProjectionMatrix(lightViewMatrix, bounds);
    _lightSpaceMatrix = lightProjectionMatrix * lightViewMatrix;
    // https://learn.microsoft.com/en-us/windows/win32/dxtecharts/common-techniques-to-improve-shadow-depth-maps?redirectedfrom=MSDN
    // https://chetanjags.wordpress.com/2015/02/05/real-time-shadows-cascaded-shadow-maps/
    // https://stackoverflow.com/questions/33499053/cascaded-shadow-map-shimmering

    program.use();

    glGetIntegerv(GL_VIEWPORT, _cached_viewport);

    glViewport(0, 0, _shadowmap_size, _shadowmap_size);
    _depthFBO.bind();
    glClear(GL_DEPTH_BUFFER_BIT);

    return _lightSpaceMatrix;
}

void Shadowmap::end()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(_cached_viewport[0], _cached_viewport[1], _cached_viewport[2], _cached_viewport[3]);
}

void Shadowmap::setSunDir(const glm::vec3& sunDir)
{
    _sunDir = sunDir;
}

glm::mat4 Shadowmap::getLighViewMatrix(const std::vector<glm::vec3>& cameraFrustumCorners, const glm::vec3& lightDir)
{
    glm::vec3 center = glm::vec3(0, 0, 0);
    for (const auto& v : cameraFrustumCorners) {
        center += v;
    }
    center /= cameraFrustumCorners.size();

    return glm::lookAt(
        center + lightDir,
        center,
        glm::vec3(0.0f, 1.0f, 0.0f)
    );
}

FrustumBounds Shadowmap::computeFrustumBounds(const glm::mat4& lightView, const std::vector<glm::vec3>& corners)
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
        const glm::vec4 trf = lightView * glm::vec4(v, 1.0f);
        b.minX = glm::min(b.minX, trf.x);
        b.maxX = glm::max(b.maxX, trf.x);
        b.minY = glm::min(b.minY, trf.y);
        b.maxY = glm::max(b.maxY, trf.y);
        b.minZ = glm::min(b.minZ, trf.z);
        b.maxZ = glm::max(b.maxZ, trf.z);
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
