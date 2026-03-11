#pragma once

#include "Camera.hpp"

class FPSCamera: public Camera {
    public:
        FPSCamera()
        {}
        FPSCamera(const glm::vec3& position, float yaw, float pitch, float fov, float aspect_ratio, float near_plane, float far_plane, bool reversedZ_enabled=false):
            Camera(fov, aspect_ratio, near_plane, far_plane, reversedZ_enabled),
            m_position(position),
            m_yaw(yaw),
            m_pitch(pitch)
        {
            m_smoothPosition = position;
            m_smoothYaw = yaw;
            m_smoothPitch = pitch;
        }

        glm::mat4 getView() const;
        glm::mat4 getViewLocal() const;

        glm::vec3 right() const;
        glm::vec3 up() const;
        glm::vec3 forward() const;


        float getYaw() const;
        float getPitch() const;

        glm::dvec3 getPosition() const;

        void setPosition(const glm::dvec3& p);
        void update(double dt);
        void move(const glm::vec3& direction);
        void onMouseMotion(int x, int y, int dx, int dy);
        void setSpeed(float value);

    private:
        void _updateVectors();

    private:
        float m_speed = 10.0f;
        float m_mouseSensitivity = 0.002f;

        glm::dvec3 m_movement = {0.0f, 0.0f, 0.0f}; // reset each frame

        glm::dvec3 m_position = {0.0f, 0.0f, 0.0f};
        float m_yaw = 0.0f;
        float m_pitch = 0.0f;
        float m_roll = 0.0f;

        float m_smoothYaw = 0.0f;
        float m_smoothPitch = 0.0f;
        float m_smoothRoll = 0.0f;
        glm::dvec3 m_smoothPosition = {0.0f, 0.0f, 0.0f};

        glm::vec3 m_worldUp = {0.0, 1.0, 0.0};
        glm::vec3 m_up = {0.0, 1.0, 0.0};
        glm::vec3 m_right = {1.0, 0.0, 0.0};
        glm::vec3 m_forward = {0.0, 0.0, 1.0};

    public:
};
