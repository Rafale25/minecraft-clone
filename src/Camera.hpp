#pragma once

#include <iostream>
#include <math.h>

#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Camera {
    public:
        Camera() {}
        Camera(float fov, float aspect_ratio, float near_plane, float far_plane):
            fov(fov), aspect_ratio(aspect_ratio), near_plane(near_plane), far_plane(far_plane)
        {}

        glm::mat4 getProjection() {
            return glm::perspective(glm::radians(fov), aspect_ratio, near_plane, far_plane);
        }

        virtual glm::mat4 getView() = 0;
        virtual glm::vec3 getPosition() = 0;

    public:
        // glm::mat4 projection;
        // glm::mat4 view;

        float fov = 60.0f;
        float aspect_ratio = 16.0f / 9.0f;
        float near_plane = 0.1f;
        float far_plane = 1000.0f;
};

class OrbitCamera: public Camera {
    public:
        OrbitCamera():
            _center(glm::vec3(0.0f)), _yaw(0.0f), _pitch(0.0f), _distance(5.0f)
        {}
        OrbitCamera(glm::vec3 center, float angle, float pitch, float distance):
            _center(center), _yaw(angle), _pitch(pitch), _distance(distance)
        {}
        OrbitCamera(glm::vec3 center, float angle, float pitch, float distance, float fov, float aspect_ratio, float near_plane, float far_plane):
            _center(center), _yaw(angle), _pitch(pitch), _distance(distance),
            Camera(fov, aspect_ratio, near_plane, far_plane)
        {}

        glm::mat4 getView() {
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

        glm::vec3 getPosition() {
            glm::vec3 eye = glm::vec3(1.0f, 0.0f, 0.0f);

            eye = glm::rotateZ(eye, _pitch);
            eye = glm::rotateY(eye, _yaw);
            eye *= _distance;

            return eye;
        }

        float getYaw() const {
            return _yaw;
        }

        float getPitch() const {
            return _pitch;
        }

        void setYaw(float value) {
            _yaw = value;
        }

        void setPitch(float value) {
            if (value > M_PI_2)
                value = M_PI_2 - 0.01f;
            if (value < -M_PI_2)
                value = -M_PI_2 + 0.01f;

            _pitch = value;
        }

        float getDistance() const {
            return _distance;
        }

        void setDistance(float value) {
            if (value < 0.01f)
                value = 0.01f;
            if (value > 1000.0f)
                value = 1000.0f;

            _distance = value;
        }

        void onMouseDrag(int x, int y, int dx, int dy)
        {
            setYaw( getYaw() - (dx * 0.005f) );
            setPitch( getPitch() + (dy * 0.005f) );
        }

        void onMouseScroll(int scroll_x, int scroll_y)
        {
            setDistance( getDistance() - (scroll_y * 0.2f) );
        }

    private:
        glm::vec3 _center;
        float _yaw;
        float _pitch;
        float _distance;
};

class FPSCamera: public Camera {
    public:
        FPSCamera()
        {}
        FPSCamera(glm::vec3 position, float yaw, float pitch, float fov, float aspect_ratio, float near_plane, float far_plane):
            _position(position),
            _yaw(yaw),
            _pitch(pitch),
            Camera(fov, aspect_ratio, near_plane, far_plane)
        {
            _smoothPosition = position;
            _smoothYaw = yaw;
            _smoothPitch = pitch;
        }

        glm::mat4 getView()
        {
            _updateVectors();
            // return glm::lookAt(_position, _position + _forward, _up);
            return glm::lookAt(_smoothPosition, _smoothPosition + forward, _up);
        }

        glm::vec3 getPosition()
        {
            return _smoothPosition;
            // return _position;
        }

        void update(float dt)
        {
            _position += _movement * _speed * dt;
            _movement = glm::vec3(0.0f);

            _smoothYaw = glm::mix(_smoothYaw, _yaw, 0.2f);
            _smoothPitch = glm::mix(_smoothPitch, _pitch, 0.2f);
            _smoothRoll = glm::mix(_smoothRoll, _roll, 0.2f);

            _smoothPosition = glm::mix(_smoothPosition, _position, 0.15f);
        }

        void move(glm::vec3 direction)
        {
            _movement += -glm::inverse(glm::mat3(getView())) * direction;
            // _position += -glm::inverse(glm::mat3(getView())) * direction * _speed;
        }

        void onMouseMotion(int x, int y, int dx, int dy)
        {
            _yaw += (float)dx * _mouseSensitivity;
            _pitch += -(float)dy * _mouseSensitivity;

            if (_pitch > 89.0f)
                _pitch = 89.0f;
            if (_pitch < -89.0f)
                _pitch = -89.0f;
        }

    private:
        void _updateVectors()
        {
            // _forward.x = glm::cos(glm::radians(_yaw)) * glm::cos(glm::radians(_pitch));
            // _forward.y = glm::sin(glm::radians(_pitch));
            // _forward.z = glm::sin(glm::radians(_yaw)) * glm::cos(glm::radians(_pitch));

            forward.x = glm::cos(glm::radians(_smoothYaw)) * glm::cos(glm::radians(_smoothPitch));
            forward.y = glm::sin(glm::radians(_smoothPitch));
            forward.z = glm::sin(glm::radians(_smoothYaw)) * glm::cos(glm::radians(_smoothPitch));

            forward = glm::normalize(forward);
            _right = glm::normalize(glm::cross(forward, _up));
            // _up = glm::normalize(glm::cross(_right, _forward));
        }

    private:
        float _speed = 4.0f;
        float _mouseSensitivity = 0.1f;

        glm::vec3 _movement = glm::vec3(0.0f, 0.0f, 0.0); // reset each frame

        float _yaw = 0.0f;
        float _pitch = 0.0f;
        float _roll = 0.0f;
        glm::vec3 _position = glm::vec3(0.0f, 0.0f, 0.0f);

        float _smoothYaw = 0.0f;
        float _smoothPitch = 0.0f;
        float _smoothRoll = 0.0f;
        glm::vec3 _smoothPosition = glm::vec3(0.0f, 0.0f, 0.0f);


        glm::vec3 _right = glm::vec3(1.0, 0.0, 0.0);
        glm::vec3 _up = glm::vec3(0.0, 1.0, 0.0);

    public:
        glm::vec3 forward = glm::vec3(0.0, 0.0, 1.0);
};
