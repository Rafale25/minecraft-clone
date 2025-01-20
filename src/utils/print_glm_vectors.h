#pragma once

#include <glm/glm.hpp>
#include <stdio.h>

inline void printVec3(const glm::vec3& v)
{
    printf("%.3f %.3f %.3f\n", v.x, v.y, v.z);
}

inline void printVec3i(const glm::ivec3& v)
{
    printf("%d %d %d\n", v.x, v.y, v.z);
}

inline void printVec4(const glm::vec4& v)
{
    printf("%.3f %.3f %.3f %.3f\n", v.x, v.y, v.z, v.w);
}
