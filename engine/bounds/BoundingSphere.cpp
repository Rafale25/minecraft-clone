#include "BoundingSphere.hpp"
#include "Frustum.hpp"

BoundingSphere BoundingSphere::createFromFrustum(const glm::mat4& view_projection) {
    const auto corners = extractFrustumCornersWorldSpace(view_projection);

    glm::vec3 center{0.0f};
    for (const auto& p : corners) {
        center += p;
    }
    center /= corners.size();

    return {
        .center = center,
        .radius = glm::distance(center, corners[0])
    };
}
