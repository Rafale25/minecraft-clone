#include "Shadowmap.hpp"
#include "BoundingSphere.hpp"
#include "ShaderProgram.hpp"
#include "Frustum.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "VAO.hpp"

static const float borderColor[4] = {1.0f, 1.0f, 1.0f, 1.0f};

Shadowmap::Shadowmap(GLsizei shadowmap_size):
    _shadowmap_size(shadowmap_size)
    // ,_depthTexture(Texture(shadowmap_size, shadowmap_size, GL_DEPTH_COMPONENT32F, GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_BORDER, borderColor))
{
    // _depthTexture.setSwizzle({ GL_RED, GL_RED, GL_RED, GL_ONE });
    // _depthFBO.attachTexture(_depthTexture._texture, GL_DEPTH_ATTACHMENT);

    glCreateTextures(1, GL_TEXTURE_2D_ARRAY, &_depthTextureArray);
    // glTextureStorage2D(_texture, 1, format, width, height);

    glTextureParameteri(_depthTextureArray, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(_depthTextureArray, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameteri(_depthTextureArray, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTextureParameteri(_depthTextureArray, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    glTextureStorage3D(_depthTextureArray, 0, GL_DEPTH_COMPONENT32F, _shadowmap_size, _shadowmap_size, shadowCascadeLevels.size() + 1);

    constexpr float bordercolor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTextureParameterfv(_depthTextureArray, GL_TEXTURE_BORDER_COLOR, borderColor);

    _matricesBuffer = createBufferStorage(NULL, 4*16 * shadowCascadeLevels.size());
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, _matricesBuffer);
}

// need to extract the boundingSphere + shimmering fix code out of begin to make it simpler

glm::mat4 Shadowmap::begin(const glm::mat4& projection, const glm::mat4& view, const ShaderProgram &program)
{
    BoundingSphere sphere = BoundingSphere::createFromFrustum(projection * view);
    sphere.radius = glm::ceil(sphere.radius); // fix micro shimmering cause by radius changing by very tiny amount

    auto corners = extractFrustumCornersWorldSpace(projection * view);
    glm::mat4 lightViewMatrix = getLightViewMatrix(corners, _sunDir);

    const float extraBackup = 20.0f;
    const float nearClip = 1.0f;
    float backupDist = extraBackup + nearClip + sphere.radius;

    float bounds = sphere.radius * 2.0f;
    float farClip = extraBackup + sphere.radius;

    glm::mat4 lightProjectionMatrix = glm::orthoZO(-bounds*0.5f, bounds*0.5f, -bounds*0.5f, bounds*0.5f, nearClip, farClip);


    // shimmering fix // https://stackoverflow.com/questions/33499053/cascaded-shadow-map-shimmering
    // Create the rounding matrix, by projecting the world-space origin and determining
    // the fractional offset in texel space
    glm::mat4 shadowMatrix = lightProjectionMatrix * lightViewMatrix;
    glm::vec4 shadowOrigin = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    shadowOrigin = shadowMatrix * shadowOrigin;
    shadowOrigin = shadowOrigin * (_shadowmap_size / 2.0f);

    glm::vec4 roundedOrigin = glm::round(shadowOrigin);
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

glm::mat4 Shadowmap::getLightViewMatrix(const std::vector<glm::vec3>& cameraFrustumCorners, const glm::vec3& lightDir)
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

#include "Camera.hpp"

glm::mat4 Shadowmap::getLightSpaceMatrix(const Camera& camera, float near_plane, float far_plane)
{
    const glm::mat4 proj = glm::perspective(glm::radians(camera.fov), camera.aspect_ratio, near_plane, far_plane);

    auto corners = extractFrustumCornersWorldSpace(proj * camera.getView());
    glm::mat4 lightViewMatrix = getLightViewMatrix(corners, _sunDir);
    FrustumBounds bounds = computeFrustumBounds(lightViewMatrix, corners);
    glm::mat4 lightProjectionMatrix = getLightProjectionMatrix(bounds);

    return lightProjectionMatrix;
}

std::vector<glm::mat4> Shadowmap::getLightSpaceMatrices(const Camera& camera)
{
    float near_plane = 0.1f;
    float far_plane = 1000.0f;

    std::vector<glm::mat4> ret;
    for (size_t i = 0; i < shadowCascadeLevels.size() + 1; ++i)
    {
        if (i == 0)
        {
            ret.push_back(getLightSpaceMatrix(camera, near_plane, shadowCascadeLevels[i]));
        }
        else if (i < shadowCascadeLevels.size())
        {
            ret.push_back(getLightSpaceMatrix(camera, shadowCascadeLevels[i - 1], shadowCascadeLevels[i]));
        }
        else
        {
            ret.push_back(getLightSpaceMatrix(camera, shadowCascadeLevels[i - 1], far_plane));
        }
    }
    return ret;
}
