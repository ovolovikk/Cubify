#ifndef GLFW_INPUT_CONTROLLER_HPP
#define GLFW_INPUT_CONTROLLER_HPP

#include "Core/Input/IInputController.hpp"

struct GLFWwindow;

class GLFWInputController : public IInputController
{
public:
    GLFWInputController(GLFWwindow* window);

    bool isMovingForward() const override;
    bool isMovingBackward() const override;
    bool isMovingLeft() const override;
    bool isMovingRight() const override;
    bool isMovingUp() const override;
    bool isMovingDown() const override;
    bool isSprinting() const override;

    glm::vec2 getMouseDelta() const override;
    bool isMouseButtonPressed(int button) const override;

    bool isKeyPressed(int key) const override;
    void update() override;

private:
    GLFWwindow* window;
    double last_x, last_y;
    double delta_x, delta_y;
    bool first_mouse;
    
};

#endif // GLFW_INPUT_CONTROLLER
