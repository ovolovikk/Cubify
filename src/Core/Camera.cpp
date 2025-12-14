#include "Core/Camera.hpp"

#include "glm/gtc/matrix_transform.hpp"

#include "Core/Input/IInputController.hpp"

Camera::Camera(vec3 _position, float _fov, float _aspect)
    : position(_position), fov(_fov), aspect(_aspect), 
      yaw(-90.0f), pitch(0.0f), worldUp(glm::vec3(0.0f, 1.0f, 0.0f))
{
    front = vec3(0.0f, 0.0f, -1.0f);
    up = vec3(0.0f, 1.0f, 0.0f);
    updateCameraVectors();
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

void Camera::SetAspect(float aspect_)
{
    aspect = aspect_;
}

void Camera::processInput(const IInputController& input, float deltaTime)
{
    float velocity = MOVEMENT_SPEED * deltaTime;
    if (input.isSprinting()) velocity *= 2.0f;

    if (input.isMovingForward())  position += front * velocity;
    if (input.isMovingBackward()) position -= front * velocity;
    if (input.isMovingLeft())     position -= right * velocity;
    if (input.isMovingRight())    position += right * velocity;
    if (input.isMovingUp())       position += worldUp * velocity;
    if (input.isMovingDown())     position -= worldUp * velocity;

    glm::vec2 mouseDelta = input.getMouseDelta();
    processMouseMovement(mouseDelta.x, mouseDelta.y);
}

void Camera::processMouseMovement(float xoffset, float yoffset, bool constrainPitch)
{
    xoffset *= MOUSE_SENSITIVITY;
    yoffset *= MOUSE_SENSITIVITY;

    yaw   += xoffset;
    pitch += yoffset;

    if (constrainPitch)
    {
        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;
    }

    updateCameraVectors();
}

void Camera::updateCameraVectors()
{
    glm::vec3 newFront;
    newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch)); 
    newFront.y = sin(glm::radians(pitch));
    newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(newFront);

    right = glm::normalize(glm::cross(front, worldUp));
    up    = glm::normalize(glm::cross(right, front));
}
