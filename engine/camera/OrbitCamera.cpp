#include "OrbitCamera.hpp"
#include <glm/gtx/rotate_vector.hpp>

glm::mat4 OrbitCamera::getView() const {
    glm::vec3 eye = glm::vec3(1.0f, 0.0f, 0.0f);

    eye = glm::rotateZ(eye, m_pitch);
    eye = glm::rotateY(eye, m_yaw);
    eye *= m_distance;

    glm::mat4 view = glm::lookAt(
        eye,
        m_center,
        glm::vec3(0.f, 1.f, 0.f)
    );
    return view;
}

glm::vec3 OrbitCamera::getPosition() const {
    glm::vec3 eye = glm::vec3(1.0f, 0.0f, 0.0f);

    eye = glm::rotateZ(eye, m_pitch);
    eye = glm::rotateY(eye, m_yaw);
    eye *= m_distance;

    return eye;
}

float OrbitCamera::getYaw() const {
    return m_yaw;
}

float OrbitCamera::getPitch() const {
    return m_pitch;
}

void OrbitCamera::setYaw(float value) {
    m_yaw = value;
}

void OrbitCamera::setPitch(float value) {
    if (value > (glm::pi<float>() / 2.0))
        value = (glm::pi<float>() / 2.0) - 0.01f;
    if (value < -(glm::pi<float>() / 2.0))
        value = -(glm::pi<float>() / 2.0) + 0.01f;

    m_pitch = value;
}

float OrbitCamera::getDistance() const {
    return m_distance;
}

void OrbitCamera::setDistance(float value) {
    if (value < 0.01f)
        value = 0.01f;
    if (value > 1000.0f)
        value = 1000.0f;

    m_distance = value;
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
