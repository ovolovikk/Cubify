#ifndef CAMERA_H
#define CAMERA_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

using glm::mat4;
using glm::vec3;

class Camera
{
public:
    Camera(vec3 _position, float _fov = 45.f, float _aspect = 4.f/3.f);

    mat4 GetProjectionMatrix() const;
    mat4 GetViewMatrix() const;

    void SetPosition(vec3 position);
    void LookAt(vec3 target);
    
    void MoveForward();
    void MoveBackward();
    void MoveLeft();
    void MoveRight();
    void MoveUp();
    void MoveDown();
    
    void ProcessWASDMovement(GLFWwindow* window);
    void ProcessMouseMovement(GLFWwindow* window, float deltaTime);

private:
    vec3 position;
    vec3 front;
    vec3 up;

    float fov;
    float aspect;
    float near_plane = 0.1f;
    float far_plane = 100.f;

    float yaw = -90.0f; // vertical
    float pitch = 0.0f; // horizontal
    
    const float MOVEMENT_SPEED = 0.05f;
    const float MOUSE_SENSITIVITY = 0.1f;
    
    void updateCameraVectors();
};

#endif // CAMERA_H
