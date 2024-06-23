#include "fps_camera.hpp"

// #include <cmath>
#include <glm/gtc/matrix_transform.hpp>

#include "lerp.hpp"

glm::mat4 FPSCamera::getView()
{
    _updateVectors();
    return glm::lookAt(_smoothPosition, _smoothPosition + forward, _up);
}

float FPSCamera::getYaw()
{
    return (_smoothYaw - M_PI_2);
}

float FPSCamera::getPitch()
{
    return (_smoothPitch);
}

glm::vec3 FPSCamera::getPosition()
{
    return _smoothPosition;
}

void FPSCamera::setPosition(glm::vec3 p)
{
    _position = p;
    _smoothPosition = p;
}


void FPSCamera::update(float dt)
{
    _position += _movement * _speed * dt;
    _movement = glm::vec3(0.0f);

    _smoothYaw = expDecay(_smoothYaw, _yaw, 50.0f, dt);
    _smoothPitch = expDecay(_smoothPitch, _pitch, 50.0f, dt);
    _smoothRoll = expDecay(_smoothRoll, _roll, 50.0f, dt);

    _smoothPosition = expDecay(_smoothPosition, _position, 16.0f, dt);
}

void FPSCamera::move(glm::vec3 direction)
{
    // _movement += -glm::inverse(glm::mat3(getView())) * direction;

    glm::mat4 rotateM = glm::rotate(glm::mat4(1.0f), -getYaw(), {0.0f, 1.0f, 0.0f});
    direction = glm::vec3(rotateM * glm::vec4(direction, 1.0f));
    direction.y = -direction.y;

    _movement += direction;
}

void FPSCamera::onMouseMotion(int x, int y, int dx, int dy)
{
    _yaw += (float)dx * _mouseSensitivity; // TODO: Do modulo on this value
    _pitch += -(float)dy * _mouseSensitivity;

    const float epsilon = 0.001f;
    _pitch = std::clamp(_pitch, (float)-M_PI_2 + epsilon, (float)M_PI_2 - epsilon);
}

void FPSCamera::setSpeed(float value)
{
    _speed = value;
}

void FPSCamera::_updateVectors()
{
    forward.x = glm::cos(_smoothYaw) * glm::cos(_smoothPitch);
    forward.y = glm::sin(_smoothPitch);
    forward.z = glm::sin(_smoothYaw) * glm::cos(_smoothPitch);

    forward = glm::normalize(forward);
    _right = glm::normalize(glm::cross(forward, _up));
}
