#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>

class IInputController;

using glm::mat4;
using glm::vec3;

class Camera
{
public:
    Camera(vec3 _position, float _fov = 45.f, float _aspect = 4.f/3.f);

    mat4 GetProjectionMatrix() const;
    mat4 GetViewMatrix() const;


    vec3 GetPosition() const { return position; }
    void SetPosition(vec3 position);
    void SetAspect(float aspect);
    
    void processInput(const IInputController& input, float DeltaTime);

private:
    vec3 position;
    vec3 front;
    vec3 up;
    vec3 right;
    vec3 worldUp;

    float fov;
    float aspect;
    float near_plane = 0.1f;
    float far_plane = 1000.f;

    float yaw; // vertical
    float pitch; // horizontal
    
    const float MOVEMENT_SPEED = 60.0f;
    const float MOUSE_SENSITIVITY = 0.1f;
    
    void updateCameraVectors();
};

#endif // CAMERA_H
