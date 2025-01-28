#include "Transform.hpp"

glm::mat4 Transform::getMatrix() const
{
    glm::mat4 m = glm::mat4(1.0f);

    m = glm::translate(m, this->position);
    m = m * glm::toMat4(rotation);
    m = glm::scale(m, this->scale);

    return m;
}
