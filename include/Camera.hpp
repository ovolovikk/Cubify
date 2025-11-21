#ifndef CAMERA_H
#define CAMERA_H

#include <GL/glew.h>
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
    
    void MoveForward(float distance);
    void MoveBackward(float distance);
    void MoveLeft(float distance);
    void MoveRight(float distance);
    void MoveUp(float distance);
    void MoveDown(float distance);

private:
    vec3 position;
    vec3 front;
    vec3 up;

    float fov;
    float aspect;
    float near_plane = 0.1f;
    float far_plane = 100.f;
};

#endif // CAMERA_H
