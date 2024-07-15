#include "camera.hpp"

class FPSCamera: public Camera {
    public:
        FPSCamera()
        {}
        FPSCamera(const glm::vec3& position, float yaw, float pitch, float fov, float aspect_ratio, float near_plane, float far_plane):
            Camera(fov, aspect_ratio, near_plane, far_plane),
            _position(position),
            _yaw(yaw),
            _pitch(pitch)
        {
            _smoothPosition = position;
            _smoothYaw = yaw;
            _smoothPitch = pitch;
        }

        glm::mat4 getView() const;

        glm::vec3 right() const;
        glm::vec3 up() const;
        glm::vec3 forward() const;


        float getYaw() const;
        float getPitch() const;
        glm::vec3 getPosition() const;
        void setPosition(const glm::vec3& p);
        void update(float dt);
        void move(const glm::vec3& direction);
        void onMouseMotion(int x, int y, int dx, int dy);
        void setSpeed(float value);

    private:
        void _updateVectors();

    private:
        float _speed = 10.0f;
        float _mouseSensitivity = 0.002f;

        glm::vec3 _movement = {0.0f, 0.0f, 0.0f}; // reset each frame

        glm::vec3 _position = {0.0f, 0.0f, 0.0f};
        float _yaw = 0.0f;
        float _pitch = 0.0f;
        float _roll = 0.0f;

        float _smoothYaw = 0.0f;
        float _smoothPitch = 0.0f;
        float _smoothRoll = 0.0f;
        glm::vec3 _smoothPosition = {0.0f, 0.0f, 0.0f};

        glm::vec3 _world_up = {0.0, 1.0, 0.0};
        glm::vec3 _up = {0.0, 1.0, 0.0};
        glm::vec3 _right = {1.0, 0.0, 0.0};
        glm::vec3 _forward = {0.0, 0.0, 1.0};

    public:
};
