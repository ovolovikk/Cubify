#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
using glm::mat4;

class Camera
{
public:
    Camera(mat4 projection_mat, mat4 model_mat, mat4 view_mat);

private:
    mat4 ProjectionMat;
    mat4 ModelMat;
    mat4 ViewMat;
    mat4 MVP;
};

#endif // CAMERA_H
