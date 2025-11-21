#include "Camera.hpp"
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera(vec3 _position, float _fov, float _aspect)
: position(_position), fov(_fov), aspect(_aspect)
{
    front = vec3(0.0f, 0.0f, -1.0f);
    up = vec3(0.0f, 1.0f, 0.0f);
}

mat4 Camera::GetProjectionMatrix() const
{
    return glm::perspective(glm::radians(fov), aspect, near_plane, far_plane);
}

mat4 Camera::GetViewMatrix() const
{
    return glm::lookAt(position, position + front, up);
}

void Camera::SetPosition(vec3 new_position)
{
    position = new_position;
}

void Camera::LookAt(vec3 target)
{
    front = glm::normalize(target - position);
}

void Camera::MoveForward(float distance)
{
    position += front * distance;
}

void Camera::MoveBackward(float distance)
{
    position -= front * distance;
}

void Camera::MoveLeft(float distance)
{
    vec3 right = glm::normalize(glm::cross(front, up));
    position -= right * distance;
}

void Camera::MoveRight(float distance)
{
    vec3 right = glm::normalize(glm::cross(front, up));
    position += right * distance;
}

void Camera::MoveUp(float distance)
{
    position += up * distance;
}

void Camera::MoveDown(float distance)
{
    position -= up * distance;
}
