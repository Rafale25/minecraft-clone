#include "camera.hpp"

class FPSCamera: public Camera {
    public:
        FPSCamera()
        {}
        FPSCamera(glm::vec3 position, float yaw, float pitch, float fov, float aspect_ratio, float near_plane, float far_plane):
            Camera(fov, aspect_ratio, near_plane, far_plane),
            _position(position),
            _yaw(yaw),
            _pitch(pitch)
        {
            _smoothPosition = position;
            _smoothYaw = yaw;
            _smoothPitch = pitch;
        }

        glm::mat4 getView();

        float getYaw();
        float getPitch();
        glm::vec3 getPosition();
        void setPosition(glm::vec3 p);
        void update(float dt);
        void move(glm::vec3 direction);
        void onMouseMotion(int x, int y, int dx, int dy);
        void setSpeed(float value);

    private:
        void _updateVectors();

    private:
        float _speed = 10.0f;
        float _mouseSensitivity = 0.002f;

        glm::vec3 _movement = glm::vec3(0.0f, 0.0f, 0.0); // reset each frame

        glm::vec3 _position = glm::vec3(0.0f, 0.0f, 0.0f);
        float _yaw = 0.0f;
        float _pitch = 0.0f;
        float _roll = 0.0f;

        float _smoothYaw = 0.0f;
        float _smoothPitch = 0.0f;
        float _smoothRoll = 0.0f;
        glm::vec3 _smoothPosition = glm::vec3(0.0f, 0.0f, 0.0f);


        glm::vec3 _right = glm::vec3(1.0, 0.0, 0.0);
        glm::vec3 _up = glm::vec3(0.0, 1.0, 0.0);

    public:
        glm::vec3 forward = glm::vec3(0.0, 0.0, 1.0);
};
