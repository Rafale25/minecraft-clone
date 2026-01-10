#include "FpsCamera.hpp"
#include "lerp.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/common.hpp>

glm::mat4 FPSCamera::getView() const
{
    return glm::lookAt(m_smoothPosition, m_smoothPosition + m_forward, m_worldUp);
}

float FPSCamera::getYaw() const
{
    return (m_smoothYaw - glm::pi<float>() / 2.0);
}

float FPSCamera::getPitch() const
{
    return (m_smoothPitch);
}

glm::vec3 FPSCamera::getPosition() const
{
    return m_smoothPosition;
}

void FPSCamera::setPosition(const glm::vec3& p)
{
    m_position = p;
    m_smoothPosition = p;
}


void FPSCamera::update(float dt)
{
    m_position += m_movement * m_speed * dt;
    m_movement = glm::vec3(0.0f);

    m_smoothYaw = expDecay(m_smoothYaw, m_yaw, 50.0f, dt);
    m_smoothPitch = expDecay(m_smoothPitch, m_pitch, 50.0f, dt);
    m_smoothRoll = expDecay(m_smoothRoll, m_roll, 50.0f, dt);

    m_smoothPosition = expDecay(m_smoothPosition, m_position, 16.0f, dt);

    _updateVectors();
}

void FPSCamera::move(const glm::vec3& direction)
{
    glm::mat4 rotateM = glm::rotate(glm::mat4(1.0f), -getYaw(), {0.0f, 1.0f, 0.0f});
    glm::vec3 dir = glm::vec3(rotateM * glm::vec4(direction, 1.0f));
    dir.y = -dir.y;

    m_movement += dir;
}

void FPSCamera::onMouseMotion(int x, int y, int dx, int dy)
{
    m_yaw += (float)dx * m_mouseSensitivity; // TODO: Do modulo on this value
    m_pitch += -(float)dy * m_mouseSensitivity;

    const float epsilon = 0.001f;
    m_pitch = glm::clamp(m_pitch, (float)-(glm::pi<float>() / 2.0) + epsilon, (float)(glm::pi<float>() / 2.0) - epsilon);
}

void FPSCamera::setSpeed(float value)
{
    m_speed = value;
}

glm::vec3 FPSCamera::right() const {
    return m_right;
}

glm::vec3 FPSCamera::up() const {
    return m_up;
}

glm::vec3 FPSCamera::forward() const {
    return m_forward;
}

void FPSCamera::_updateVectors()
{
    m_forward.x = glm::cos(m_smoothYaw) * glm::cos(m_smoothPitch);
    m_forward.y = glm::sin(m_smoothPitch);
    m_forward.z = glm::sin(m_smoothYaw) * glm::cos(m_smoothPitch);

    m_forward = glm::normalize(m_forward);
    m_right = glm::normalize(glm::cross(m_forward, m_worldUp));
    m_up = glm::normalize(glm::cross(m_right, m_forward));
}
