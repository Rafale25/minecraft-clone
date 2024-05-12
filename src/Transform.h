#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

struct Transform
{
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);

    glm::mat4 getMatrix(void)
    {
        glm::mat4 m = glm::mat4(1.0f);

        m = glm::translate(m, this->position);
        m = m * glm::toMat4(rotation);
        m = glm::scale(m, this->scale);

        return m;
    }
};
