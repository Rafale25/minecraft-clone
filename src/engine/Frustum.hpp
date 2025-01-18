#pragma once

#include <glm/glm.hpp>

struct Frustum
{
	glm::vec4 planes[6];
};

struct AABB {
	glm::vec3 min, max;

	bool isOnFrustum(const Frustum& frustum) const;
};

class Camera;

Frustum createFrustumFromCamera(const Camera& camera);
Frustum createFrustumFromViewProjection(const glm::mat4& view_projection);
void extractPlanesFromProjectionViewMatrix(const glm::mat4& m, glm::vec4 planes[6]);
