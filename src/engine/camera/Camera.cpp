#include "Camera.hpp"

#include <glm/gtx/rotate_vector.hpp>

glm::mat4 Camera::getProjection() const {
    return glm::perspective(glm::radians(fov), aspect_ratio, near_plane, far_plane);
}
