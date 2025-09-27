#pragma once

#include <glm/ext/vector_float3.hpp>
#include <glm/ext/matrix_float4x4.hpp>

struct BoundingSphere {
	glm::vec3 center;
	float radius;

    static BoundingSphere createFromFrustum(const glm::mat4& view_projection);
};
