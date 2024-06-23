#pragma once

#include "camera.hpp"

class OrbitCamera: public Camera {
public:
    OrbitCamera():
        _center(glm::vec3(0.0f)), _yaw(0.0f), _pitch(0.0f), _distance(5.0f)
    {}
    OrbitCamera(glm::vec3 center, float angle, float pitch, float distance):
        _center(center), _yaw(angle), _pitch(pitch), _distance(distance)
    {}
    OrbitCamera(glm::vec3 center, float angle, float pitch, float distance, float fov, float aspect_ratio, float near_plane, float far_plane):
        Camera(fov, aspect_ratio, near_plane, far_plane),
        _center(center), _yaw(angle), _pitch(pitch), _distance(distance)
    {}

    glm::mat4 getView();
    glm::vec3 getPosition();

    float getYaw() const;
    float getPitch() const;

    void setYaw(float value);
    void setPitch(float value);

    float getDistance() const;
    void setDistance(float value);

    void onMouseDrag(int x, int y, int dx, int dy);
    void onMouseScroll(int scroll_x, int scroll_y);

private:
    glm::vec3 _center;
    float _yaw;
    float _pitch;
    float _distance;
};
