#include "Frustum.hpp"

#include <glm/glm.hpp>
#include "transform.h"
#include "camera.hpp"

float Plane::getSignedDistanceToPlane(const glm::vec3& point) const
{
    return glm::dot(normal, point) - distance;
}

bool BoundingVolume::isOnFrustum(const Frustum& frustum) const
{
    return (isOnOrForwardPlane(frustum.leftFace) &&
            isOnOrForwardPlane(frustum.rightFace) &&
            isOnOrForwardPlane(frustum.topFace) &&
            isOnOrForwardPlane(frustum.bottomFace) &&
            isOnOrForwardPlane(frustum.nearFace) &&
            isOnOrForwardPlane(frustum.farFace));
};

//see https://gdbooks.gitbooks.io/3dcollisions/content/Chapter2/static_aabb_plane.html
bool AABB::isOnOrForwardPlane(const Plane& plane) const
{
    // Compute the projection interval radius of b onto L(t) = b.c + t * p.n
    const float r = extents.x * std::abs(plane.normal.x) + extents.y * std::abs(plane.normal.y) + extents.z * std::abs(plane.normal.z);
    return -r <= plane.getSignedDistanceToPlane(center);
}

Frustum createFrustumFromCamera(const Camera& camera, float aspect, float fovY, float zNear, float zFar)
{
    Frustum frustum;
    const float halfVSide = zFar * tanf(fovY * 0.5f);
    const float halfHSide = halfVSide * aspect;
    const glm::vec3 frontMultFar = zFar * camera.forward();

    glm::vec3 camera_position = camera.getPosition();

    frustum.nearFace = { camera_position + zNear * camera.forward(), camera.forward() };
    frustum.farFace = { camera_position + frontMultFar, -camera.forward() };
    frustum.rightFace = { camera_position, glm::cross(frontMultFar - camera.right() * halfHSide, camera.up()) };
    frustum.leftFace = { camera_position, glm::cross(camera.up(),frontMultFar + camera.right() * halfHSide) };
    frustum.topFace = { camera_position, glm::cross(camera.right(), frontMultFar - camera.up() * halfVSide) };
    frustum.bottomFace = { camera_position, glm::cross(frontMultFar + camera.up() * halfVSide, camera.right()) };

    return frustum;
}
