#include "orbit_camera.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

glm::mat4 OrbitCamera::getView() const {
    glm::vec3 eye = glm::vec3(1.0f, 0.0f, 0.0f);

    eye = glm::rotateZ(eye, _pitch);
    eye = glm::rotateY(eye, _yaw);
    eye *= _distance;

    glm::mat4 view = glm::lookAt(
        eye,
        _center,
        glm::vec3(0.f, 1.f, 0.f)
    );
    return view;
}

glm::vec3 OrbitCamera::getPosition() const {
    glm::vec3 eye = glm::vec3(1.0f, 0.0f, 0.0f);

    eye = glm::rotateZ(eye, _pitch);
    eye = glm::rotateY(eye, _yaw);
    eye *= _distance;

    return eye;
}

float OrbitCamera::getYaw() const {
    return _yaw;
}

float OrbitCamera::getPitch() const {
    return _pitch;
}

void OrbitCamera::setYaw(float value) {
    _yaw = value;
}

void OrbitCamera::setPitch(float value) {
    if (value > M_PI_2)
        value = M_PI_2 - 0.01f;
    if (value < -M_PI_2)
        value = -M_PI_2 + 0.01f;

    _pitch = value;
}

float OrbitCamera::getDistance() const {
    return _distance;
}

void OrbitCamera::setDistance(float value) {
    if (value < 0.01f)
        value = 0.01f;
    if (value > 1000.0f)
        value = 1000.0f;

    _distance = value;
}

void OrbitCamera::onMouseDrag(int x, int y, int dx, int dy)
{
    setYaw( getYaw() - (dx * 0.005f) );
    setPitch( getPitch() + (dy * 0.005f) );
}

void OrbitCamera::onMouseScroll(int scroll_x, int scroll_y)
{
    setDistance( getDistance() - (scroll_y * 0.2f) );
}
