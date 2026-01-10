#pragma once

#include <glm/detail/type_mat4x4.hpp>

class Camera {
    public:
        Camera() {}
        Camera(float fov, float aspect_ratio, float near_plane, float far_plane, bool reversedZ_enabled=false):
            fov(fov), aspectRatio(aspect_ratio), nearPlane(near_plane), farPlane(far_plane), reversedZEnabled(reversedZ_enabled)
        {}

        virtual ~Camera() = default;

        glm::mat4 getProjection() const;

        virtual glm::mat4 getView() const = 0;
        virtual glm::vec3 getPosition() const = 0;

        virtual glm::vec3 right() const = 0;
        virtual glm::vec3 up() const = 0;
        virtual glm::vec3 forward() const = 0;

    public:
        float fov = 60.0f;
        float aspectRatio = 16.0f / 9.0f;
        float nearPlane = 0.1f;
        float farPlane = 1000.0f;
        bool reversedZEnabled = false;
};
