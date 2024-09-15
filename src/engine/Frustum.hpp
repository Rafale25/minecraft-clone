#pragma once

#include <glm/glm.hpp>
#include "Transform.h"

struct Plane
{
	glm::vec3 normal = { 0.0f, 1.0f, 0.0f };
	float distance = 0.0f; // Distance with origin

	Plane() = default;
	Plane(const glm::vec3& p1, const glm::vec3& norm): normal(glm::normalize(norm)), distance(glm::dot(normal, p1)) {}

	float getSignedDistanceToPlane(const glm::vec3& point) const;
};

struct Frustum
{
    Plane topFace;
    Plane bottomFace;

    Plane rightFace;
    Plane leftFace;

    Plane farFace;
    Plane nearFace;
};

struct BoundingVolume
{
	// virtual bool isOnFrustum(const Frustum& camFrustum, const Transform& transform) const = 0;
	virtual bool isOnOrForwardPlane(const Plane& plane) const = 0;

	bool isOnFrustum(const Frustum& frustum) const;
};

struct AABB : public BoundingVolume
{
	glm::vec3 center{ 0.f, 0.f, 0.f };
    glm::vec3 extents{ 0.f, 0.f, 0.f };

    AABB(const glm::vec3& min, const glm::vec3& max)
		: BoundingVolume{}, center{ (max + min) * 0.5f }, extents{ max.x - center.x, max.y - center.y, max.z - center.z }
	{}

    //see https://gdbooks.gitbooks.io/3dcollisions/content/Chapter2/static_aabb_plane.html
	bool isOnOrForwardPlane(const Plane& plane) const final;
};

class Camera;

Frustum createFrustumFromCamera(const Camera& camera, float aspect, float fovY, float zNear, float zFar);
