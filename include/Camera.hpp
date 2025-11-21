#ifndef CAMERA_H
#define CAMERA_H

#include <GL/glew.h>
#include <glm/glm.hpp>
using glm::mat4;

class Camera
{
public:
    Camera(mat4 _projection_mat, mat4 _model_mat, mat4 _view_mat);

private:
    GLuint matrix_ID;
    mat4 projection_mat;
    mat4 model_mat;
    mat4 view_mat;
    mat4 MVP;
};

#endif // CAMERA_H
