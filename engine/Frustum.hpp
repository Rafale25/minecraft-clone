#pragma once

#include <glm/detail/type_vec4.hpp>
#include <glm/detail/type_mat4x4.hpp>

struct AABB;
class Camera;

struct Frustum
{
	glm::vec4 planes[6];
};

// Frustum createFrustumFromCamera(const Camera& camera);
Frustum createFrustumFromViewProjection(const glm::mat4& view_projection);
void extractPlanesFromProjectionViewMatrix(const glm::mat4& m, glm::vec4 planes[6]);
std::vector<glm::vec4> extractFrustumCornersWorldSpace(const glm::mat4& view_projection);
bool isAABBOnFrustum(const AABB& aabb, const Frustum& frustum);
