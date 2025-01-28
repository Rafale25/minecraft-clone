#pragma once

#include <glm/glm.hpp>

struct Frustum
{
	glm::vec4 planes[6];
};

struct AABB;
class Camera;

Frustum createFrustumFromCamera(const Camera& camera);
Frustum createFrustumFromViewProjection(const glm::mat4& view_projection);
void extractPlanesFromProjectionViewMatrix(const glm::mat4& m, glm::vec4 planes[6]);
std::vector<glm::vec4> extractFrustumCornersWorldSpace(const glm::mat4& view_projection);
bool isAABBOnFrustum(const AABB& aabb, const Frustum& frustum);
