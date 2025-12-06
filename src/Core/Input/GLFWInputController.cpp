#include "Core/Input/GLFWInputController.hpp"

#include <GLFW/glfw3.h>

GLFWInputController::GLFWInputController(GLFWwindow* window_)
    : window(window_), last_x(0), last_y(0), delta_x(0), delta_y(0), first_mouse(true)
{
    if (window)
    {
        int w, h;
        glfwGetWindowSize(window, &w, &h);
        last_x = w / 2.0;
        last_y = h / 2.0;
        glfwSetCursorPos(window, last_x, last_y);
    }
}

void GLFWInputController::update()
{
    if(!window) return;

    double curr_x, curr_y;
    glfwGetCursorPos(window, &curr_x, &curr_y);
    if (first_mouse) {
        last_x = curr_x;
        last_y = curr_y;
        first_mouse = false;
    }

    delta_x = curr_x - last_x;
    delta_y = last_y - curr_y; // Reversed since y-coordinates go from bottom to top

    last_x = curr_x;
    last_y = curr_y;
}

glm::vec2 GLFWInputController::getMouseDelta() const {
    return glm::vec2((float)delta_x, (float)delta_y);
}

bool GLFWInputController::isMovingForward() const {
    return glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;
}

bool GLFWInputController::isMovingBackward() const {
    return glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;
}

bool GLFWInputController::isMovingLeft() const {
    return glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;
}

bool GLFWInputController::isMovingRight() const {
    return glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;
}

bool GLFWInputController::isMovingUp() const {
    return glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
}

bool GLFWInputController::isMovingDown() const {
    return glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
}

bool GLFWInputController::isSprinting() const {
    return glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS;
}

bool GLFWInputController::isKeyPressed(int key) const {
    return glfwGetKey(window, key) == GLFW_PRESS;
}