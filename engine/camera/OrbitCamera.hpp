#pragma once

#include "Camera.hpp"

class OrbitCamera: public Camera {
public:
    OrbitCamera():
        m_center(glm::vec3(0.0f)), m_yaw(0.0f), m_pitch(0.0f), m_distance(5.0f)
    {}
    OrbitCamera(glm::vec3 center, float angle, float pitch, float distance):
        m_center(center), m_yaw(angle), m_pitch(pitch), m_distance(distance)
    {}
    OrbitCamera(glm::vec3 center, float angle, float pitch, float distance, float fov, float aspect_ratio, float near_plane, float far_plane):
        Camera(fov, aspect_ratio, near_plane, far_plane),
        m_center(center), m_yaw(angle), m_pitch(pitch), m_distance(distance)
    {}

    glm::mat4 getView() const;
    glm::vec3 getPosition() const;

    float getYaw() const;
    float getPitch() const;

    void setYaw(float value);
    void setPitch(float value);

    float getDistance() const;
    void setDistance(float value);

    void onMouseDrag(int x, int y, int dx, int dy);
    void onMouseScroll(int scroll_x, int scroll_y);

private:
    glm::vec3 m_center;
    float m_yaw;
    float m_pitch;
    float m_distance;
};
