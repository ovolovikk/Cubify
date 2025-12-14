#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include "Helpers/AABB.hpp"

class IInputController;

using glm::mat4;
using glm::vec3;

class Camera
{
public:
    Camera(vec3 position_, float fov_ = 60.f, float aspect_ = 16.0 / 9.f);

    mat4 GetProjectionMatrix() const;
    mat4 GetViewMatrix() const;

    vec3 GetPosition() const { return position; }
    vec3 GetFront() const { return front; }
    vec3 GetRight() const { return right; }
    float GetFOV() const { return fov; }
    void SetPosition(vec3 position);
    void SetAspect(float aspect);
    
    void processInput(const IInputController& input, float deltaTime);
    void processMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);

private:
    vec3 position;
    vec3 front;
    vec3 up;
    vec3 right;
    vec3 worldUp;

    float fov;
    float aspect;
    float near_plane;
    float far_plane;

    float yaw; // vertical
    float pitch; // horizontal
    
    static constexpr float MOVEMENT_SPEED = 10.0f;
    static constexpr float MOUSE_SENSITIVITY = 0.1f;
    
    void updateCameraVectors();
};

#endif // CAMERA_HPP
