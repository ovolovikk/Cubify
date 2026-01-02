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

    void setCursorEnabled(bool enabled) override;
    bool isCursorEnabled() const override;

private:
    GLFWwindow* window = nullptr;
    double last_x = 0, last_y = 0;
    double delta_x = 0, delta_y = 0;
    bool first_mouse = true;
    bool cursor_enabled = false;
    
};

#endif // GLFW_INPUT_CONTROLLER
