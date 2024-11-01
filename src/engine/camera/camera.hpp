#pragma once

#include <glm/glm.hpp>

class Camera {
    public:
        Camera() {}
        Camera(float fov, float aspect_ratio, float near_plane, float far_plane):
            fov(fov), aspect_ratio(aspect_ratio), near_plane(near_plane), far_plane(far_plane)
        {}

        glm::mat4 getProjection() const;

        virtual glm::mat4 getView() const = 0;
        virtual glm::vec3 getPosition() const = 0;

        virtual glm::vec3 right() const = 0;
        virtual glm::vec3 up() const = 0;
        virtual glm::vec3 forward() const = 0;

    public:
        float fov = 60.0f;
        float aspect_ratio = 16.0f / 9.0f;
        float near_plane = 0.1f;
        float far_plane = 1000.0f;
};
