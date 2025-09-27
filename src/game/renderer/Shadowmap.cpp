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



static glm::mat4 createOrthographic(float width, float height, float zNearPlane, float zFarPlane)
{
    glm::mat4 result{0.0f};

    result[0][0] = 2.0f / width;
    result[0][1] = result[0][2] = result[0][3] = 0.0f;
    result[1][1] = 2.0f / height;
    result[1][0] = result[1][2] = result[1][3] = 0.0f;
    result[2][2] = 1.0f / (zNearPlane - zFarPlane);
    result[2][0] = result[2][1] = result[2][3] = 0.0f;
    result[3][0] = result[3][1] = 0.0f;
    result[3][2] = zNearPlane / (zNearPlane - zFarPlane);
    result[3][3] = 1.0f;

    return result;
}


#include "BoundingSphere.hpp"
#include "DebugDraw.hpp"

glm::mat4 Shadowmap::begin(const glm::mat4& projection, const glm::mat4& view, const Program &program)
{
    BoundingSphere sphere = BoundingSphere::createFromFrustum(projection * view);
    sphere.radius = glm::ceil(sphere.radius); // fix micro shimmering cause by radius changing by very tiny amount

    auto corners = extractFrustumCornersWorldSpace(projection * view);
    glm::mat4 lightViewMatrix = getLighViewMatrix(corners, _sunDir);

    const float extraBackup = 20.0f;
    const float nearClip = 1.0f;
    float backupDist = extraBackup + nearClip + sphere.radius;

    float bounds = sphere.radius * 2.0f;
    float farClip = extraBackup + sphere.radius;

    glm::mat4 lightProjectionMatrix = glm::orthoZO(-bounds*0.5f, bounds*0.5f, -bounds*0.5f, bounds*0.5f, nearClip, farClip);


    // shimmering fix
    glm::mat4 shadowMatrix = lightProjectionMatrix * lightViewMatrix;
    glm::vec4 shadowOrigin = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    shadowOrigin = shadowMatrix * shadowOrigin;
    shadowOrigin = shadowOrigin * (_shadowmap_size / 2.0f);

    glm::vec4 roundedOrigin = glm::floor(shadowOrigin);
    glm::vec4 roundOffset = roundedOrigin - shadowOrigin;
    roundOffset = roundOffset * (2.0f / float(_shadowmap_size));
    roundOffset.z = 0.0f;
    roundOffset.w = 0.0f;

    glm::mat4 shadowProj = lightProjectionMatrix;
    shadowProj[3] += roundOffset;
    // --

    _lightSpaceMatrix = shadowProj * lightViewMatrix;

    // auto corners = extractFrustumCornersWorldSpace(projection * view);
    // glm::mat4 lightViewMatrix = getLighViewMatrix(corners, _sunDir);
    // FrustumBounds bounds = computeFrustumBounds(lightViewMatrix, corners);
    // glm::mat4 lightProjectionMatrix = getLightProjectionMatrix(bounds);

    // _lightSpaceMatrix = lightProjectionMatrix * lightViewMatrix;
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

glm::mat4 Shadowmap::getLightProjectionMatrix(FrustumBounds& b)
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
