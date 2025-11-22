#include "Camera.hpp"
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera(vec3 _position, float _fov, float _aspect)
: position(_position), fov(_fov), aspect(_aspect)
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

void Camera::LookAt(vec3 target)
{
    front = glm::normalize(target - position);
}

void Camera::MoveForward()
{
    position += front * MOVEMENT_SPEED;
}

void Camera::MoveBackward()
{
    position -= front * MOVEMENT_SPEED;
}

void Camera::MoveLeft()
{
    vec3 right = glm::normalize(glm::cross(front, up));
    position -= right * MOVEMENT_SPEED;
}

void Camera::MoveRight()
{
    vec3 right = glm::normalize(glm::cross(front, up));
    position += right * MOVEMENT_SPEED;
}

void Camera::MoveUp()
{
    position += up * MOVEMENT_SPEED;
}

void Camera::MoveDown()
{
    position -= up * MOVEMENT_SPEED;
}

void Camera::ProcessWASDMovement(GLFWwindow* window)
{
    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        MoveForward();
    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        MoveLeft();
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        MoveBackward();
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        MoveRight();
    if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        MoveUp();
    if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        MoveDown();
}

void Camera::ProcessMouseMovement(GLFWwindow* window, float deltaTime)
{
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    int width, height;
    glfwGetWindowSize(window, &width, &height);
    
    glfwSetCursorPos(window, width / 2.0, height / 2.0);

    float xoffset = float(xpos - (width / 2.0));
    float yoffset = float((height / 2.0) - ypos);

    xoffset *= MOUSE_SENSITIVITY;
    yoffset *= MOUSE_SENSITIVITY;

    yaw   += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    updateCameraVectors();
}

void Camera::updateCameraVectors()
{
    glm::vec3 newFront;
    newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    newFront.y = sin(glm::radians(pitch));
    newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(newFront);
}
