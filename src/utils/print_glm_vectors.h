#pragma once

#include <glm/glm.hpp>
#include <stdio.h>

void printVec3(const glm::vec3& v)
{
    printf("%.3f %.3f %.3f\n", v.x, v.y, v.z);
}

void printVec4(const glm::vec4& v)
{
    printf("%.3f %.3f %.3f %.3f\n", v.x, v.y, v.z, v.w);
}
