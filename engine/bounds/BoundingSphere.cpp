#include "BoundingSphere.hpp"
#include "Frustum.hpp"
#include "Logger.hpp"


BoundingSphere BoundingSphere::createFromFrustum(const glm::mat4& view_projection) {
    const auto corners = extractFrustumCornersWorldSpace(view_projection);

    glm::vec3 center{0.0f};
    for (const auto& p : corners) {
        center += p;
    }
    center /= corners.size();

    return {
        .center = center,
        .radius = glm::distance(center, corners[7]) // use corner 7 because it the further on of the 2 camera plane that compose the frustum
    };
}
